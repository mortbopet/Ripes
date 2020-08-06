#include "ccmanager.h"

#include "loaddialog.h"
#include "processorhandler.h"
#include "ripessettings.h"

#include <QProcess>
#include <QProgressDialog>

namespace Ripes {

const static std::vector<QString> s_validAutodetectedCCs = {"riscv64-unknown-elf-gcc", "riscv64-unknown-elf-g++",
                                                            "riscv64-unknown-elf-c++"};
const static QString s_testprogram = "int main() { return 0; }";

CCManager::CCManager() {
    if (RipesSettings::value(RIPES_SETTING_CCPATH) == "") {
        // No previous compiler path has been set. Try to autodetect a valid compiler within the current path
        const auto CCPath = tryAutodetectCC();
        if (!CCPath.isEmpty()) {
            RipesSettings::setValue(RIPES_SETTING_CCPATH, CCPath);
        }
    }

    // At startup, we will always try to validate whether the current CC set in the settings is (still) valid.
    if (RipesSettings::value(RIPES_SETTING_CCPATH) != "") {
        trySetCC(RipesSettings::value(RIPES_SETTING_CCPATH).toString());
    }
}

bool CCManager::hasValidCC() {
    return !get().m_currentCC.isEmpty();
}

QString CCManager::tryAutodetectCC() {
    for (const auto& path : s_validAutodetectedCCs) {
        QProcess process;

        process.start(path);
        process.waitForFinished();

        if (process.error() != QProcess::FailedToStart) {
            // We have detected that a valid compiler exists in path
            return path;
        }
    }

    return QString();
}

bool CCManager::trySetCC(const QString& CC) {
    const auto res = verifyCC(CC);
    if (res.success) {
        m_currentCC = CC;
    } else {
        m_currentCC = QString();
    }
    emit ccChanged(res);
    return res.success;
}

CCManager::CCRes CCManager::compileRaw(const QString& rawsource, QString outname, bool showProgressdiag) {
    // Write program to temporary file with a .c extension
    const auto tempFileTemplate =
        QString(QDir::tempPath() + QDir::separator() + QCoreApplication::applicationName() + ".XXXXXX.c");
    QTemporaryFile tmpSrcFile = QTemporaryFile(tempFileTemplate);
    tmpSrcFile.setAutoRemove(false);
    if (tmpSrcFile.open()) {
        QTextStream stream(&tmpSrcFile);
        stream << rawsource;
    }
    Q_ASSERT(!tmpSrcFile.fileName().isEmpty());
    return compile(tmpSrcFile.fileName(), outname, showProgressdiag);
}

CCManager::CCRes CCManager::compile(const QTextDocument* source, QString outname, bool showProgressdiag) {
    return compileRaw(source->toPlainText(), outname, showProgressdiag);
}

CCManager::CCRes CCManager::compile(const QString& filename, QString outname, bool showProgressdiag) {
    CCRes res;
    if (outname.isEmpty()) {
        outname = QDir::tempPath() + QDir::separator() + QCoreApplication::applicationName() + ".temp.out";
        QFile::remove(outname);  // Remove any previously compiled file
    }

    res.inFile = filename;
    res.outFile = outname;

    const auto [cc, args] = createCompileCommand(filename, outname);

    // Run compiler

    /**
     * 1. m_process will itself spawn its own thread to execute the compiler.
     * 2. QProcess should not be started in a separate QThread, so it is started in the gui thread
     * 3. We want to execute a progress dialog which may abort the QProcess.
     * 4. To facilitate all of this, we have to spin on the process state in a separate thread, to
     *    not block the execution of the progress dialog.
     */
    m_aborted = false;
    m_errored = false;
    m_process.close();
    QProgressDialog progressDiag = QProgressDialog("Executing compiler...", "Abort", 0, 0, nullptr);
    connect(&progressDiag, &QProgressDialog::canceled, &m_process, &QProcess::kill);
    connect(&progressDiag, &QProgressDialog::canceled, &m_process, [this] { m_aborted = true; });
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &progressDiag,
            &QProgressDialog::reset);
    connect(&m_process, &QProcess::errorOccurred, &progressDiag, &QProgressDialog::reset);
    connect(&m_process, &QProcess::errorOccurred, [this]() { m_errored = true; });

    m_process.start(cc, args);
    /** @todo: It is seen that if the process fails upon startup, errorOccurred executes QProgressDialog::reset.
     * However, this call does not prevent the exec() loop from running. Below we check for this case, however this does
     * not remove the race condition. Such race condition seems inherintly tied to how QDialog::exec works and no proper
     * fix has been able to be found (yet).*/
    if (!m_errored && showProgressdiag) {
        progressDiag.exec();
    }
    m_process.waitForFinished();

    const bool success = LoadDialog::validateELFFile(QFile(outname)).valid;
    res.success = success;
    res.aborted = m_aborted;

    return res;
}

QString CCManager::getError() {
    return get().m_process.readAllStandardError();
}

namespace {

QStringList sanitizedArguments(const QString& args) {
    QStringList arglist = args.split(" ");
    for (const auto invArg : {"", " ", "-"}) {
        arglist.removeAll(invArg);
    }
    return arglist;
}

}  // namespace

std::pair<QString, QStringList> CCManager::createCompileCommand(const QString& filename, const QString& outname) const {
    const auto& currentISA = ProcessorHandler::get()->currentISA();

    /**
     * @brief s_baseCC
     * Base compiler command.
     * - %1: path to compiler executable
     * - %2: machine architecture
     * - %3: machine ABI
     * - %4: user compiler arguments
     * - -x c: Enforce compilation as C language (allows us to use C++ compilers)
     * - %5: input source file
     * - %6: output executable
     * - %7: user linker arguments
     */
    const static QString s_baseCC = "%1 -march=%2 -mabi=%3 %4 -x c %5 -o %6 %7";

    QStringList compileCommand;

    // Compiler path
    compileCommand << m_currentCC;

    // Substitute machine architecture
    compileCommand << (QString("-march=") + currentISA->CCmarch());

    // Substitute machine ABI
    compileCommand << (QString("-mabi=") + currentISA->CCmabi());

    // Substitute additional CC arguments
    compileCommand << sanitizedArguments(RipesSettings::value(RIPES_SETTING_CCARGS).toString());

    // Enforce compilation as C language (allows us to use C++ compilers)
    compileCommand << "-x"
                   << "c";

    // Substitute in and out files
    compileCommand << filename << "-o" << outname;

    // Substitute additional linker arguments
    compileCommand << sanitizedArguments(RipesSettings::value(RIPES_SETTING_LDARGS).toString());

    return {compileCommand[0], compileCommand.mid(1)};
}

CCManager::CCRes CCManager::verifyCC(const QString& CCPath) {
    // Try to set CCPath as current compiler, and compile test program
    m_currentCC = CCPath;

    CCRes res;
    const auto compilerExecInfo = QFileInfo(m_currentCC);
    auto abs = compilerExecInfo.absolutePath();

    if (m_currentCC.isEmpty()) {
        res.errorMessage = "No path specified";
        res.success = false;
        goto verifyCC_end;
    }

    if (!compilerExecInfo.isExecutable()) {
        res.errorMessage = "'" + m_currentCC + "' is not an executable file.";
        res.success = false;
        goto verifyCC_end;
    }

    res = compileRaw(s_testprogram, QString(), false);

    if (!res.success) {
        res.errorMessage += "Failed to compile test program.\n";
        res.errorMessage += "Compiler output:\n";
        res.errorMessage += "\tstdout: " + QString(m_process.readAllStandardOutput()) + "\n";
        res.errorMessage += "\tstderr: " + QString(m_process.readAllStandardError()) + "\n";
    }

    // Cleanup
    res.clean();

verifyCC_end:
    return res;
}

}  // namespace Ripes
