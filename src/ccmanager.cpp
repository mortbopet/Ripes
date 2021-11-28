#include "ccmanager.h"

#include "io/iomanager.h"
#include "loaddialog.h"
#include "processorhandler.h"
#include "ripessettings.h"
#include "utilities/systemutils.h"

#include <QProcess>
#include <QProgressDialog>
#include <QTextDocument>

namespace Ripes {

const static std::vector<QString> s_validAutodetectedCCs = {"riscv64-unknown-elf-gcc", "riscv64-unknown-elf-g++",
                                                            "riscv64-unknown-elf-c++"};
const static QString s_testprogram = "int main() { return 0; }";

QString indentString(const QString& string, int indent) {
    auto subStrings = string.split("\n");
    auto indentedStrings = QStringList();
    for (const auto& str : qAsConst(subStrings)) {
        indentedStrings << QString(" ").repeated(indent) + str;
    }
    return indentedStrings.join("\n");
}

QString CCManager::CompileError::toString(const CompileCommand& cc) const {
    QStringList out;
    auto indentAdd = [&](const QString& msg, const QString& indentMsg) {
        if (!indentMsg.isEmpty()) {
            out << msg;
            out << indentString(indentMsg, 4);
        }
    };

    indentAdd("Compilation failed", errMsg);
    indentAdd("Compile command was:", cc.toString());
    indentAdd("\nstdout output:", _stdout);
    indentAdd("\nstderr output:", _stderr);
    return out.join("\n");
}

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
    auto ccIt = llvm::find_if(s_validAutodetectedCCs, [&](const QString& path) { return isExecutable(path); });
    if (ccIt != s_validAutodetectedCCs.end())
        return *ccIt;
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
    if (!(m_tmpSrcFile && (QFile::exists(m_tmpSrcFile->fileName())))) {
        const auto tempFileTemplate =
            QString(QDir::tempPath() + QDir::separator() + QCoreApplication::applicationName() + ".XXXXXX.c");
        QTemporaryFile tmpSrcFile = QTemporaryFile(tempFileTemplate);
        tmpSrcFile.setAutoRemove(false);
        if (tmpSrcFile.open()) {
            m_tmpSrcFile = std::make_unique<QFile>(tmpSrcFile.fileName());
        }
    }

    Q_ASSERT(m_tmpSrcFile);
    if (m_tmpSrcFile->open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        m_tmpSrcFile->write(rawsource.toUtf8());
        m_tmpSrcFile->close();
    }

    QStringList sourceFiles;
    sourceFiles << m_tmpSrcFile->fileName();

    // Include peripheral header file, if available
    const QString peripheralSymbolsHeader = IOManager::get().cSymbolsHeaderpath();
    if (!peripheralSymbolsHeader.isEmpty()) {
        sourceFiles << peripheralSymbolsHeader;
    }

    return compile(sourceFiles, outname, showProgressdiag);
}

CCManager::CCRes CCManager::compile(const QTextDocument* source, QString outname, bool showProgressdiag) {
    return compileRaw(source->toPlainText(), outname, showProgressdiag);
}

CCManager::CCRes CCManager::compile(const QStringList& files, QString outname, bool showProgressdiag) {
    CCRes res;
    if (outname.isEmpty()) {
        outname = QDir::tempPath() + QDir::separator() + QCoreApplication::applicationName() + ".temp.out";
        QFile::remove(outname);  // Remove any previously compiled file
    }

    res.inFiles = files;
    res.outFile = outname;

    const auto cc = createCompileCommand(files, outname);
    res.cc = cc;

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
    m_process.setWorkingDirectory(cc.bin.absolutePath());
    m_process.setProgram(cc.bin.absoluteFilePath());
    m_process.setArguments(cc.args);

    m_process.start();
    /** @todo: It is seen that if the process fails upon startup, errorOccurred executes QProgressDialog::reset.
     * However, this call does not prevent the exec() loop from running. Below we check for this case, however this does
     * not remove the race condition. Such race condition seems inherintly tied to how QDialog::exec works and no proper
     * fix has been able to be found (yet).*/
    if (!m_errored && showProgressdiag) {
        progressDiag.exec();
    }
    m_process.waitForFinished();

    auto elfInfo = LoadDialog::validateELFFile(QFile(outname));
    res.success = elfInfo.valid;
    res.errorOutput.errMsg = elfInfo.errorMessage;
    res.aborted = m_aborted;

    return res;
}

QString CCManager::getError() {
    return get().m_process.readAllStandardError();
}

static QStringList sanitizedArguments(const QString& args) {
    QStringList arglist = args.split(" ");
    for (const auto invArg : {"", " ", "-"}) {
        arglist.removeAll(invArg);
    }
    return arglist;
}

CCManager::CompileCommand CCManager::createCompileCommand(const QStringList& files, const QString& outname) const {
    const auto& currentISA = ProcessorHandler::currentISA();

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
    QStringList compileCommand;
    CompileCommand cc;

    // Compiler path
    cc.bin = QFileInfo(m_currentCC);

    // Substitute machine architecture
    cc.args << (QString("-march=") + currentISA->CCmarch());

    // Substitute machine ABI
    cc.args << (QString("-mabi=") + currentISA->CCmabi());

    // Substitute additional CC arguments
    cc.args << sanitizedArguments(RipesSettings::value(RIPES_SETTING_CCARGS).toString());

    // Enforce compilation as C language (allows us to use C++ compilers)
    cc.args << "-x"
            << "c";

    // Substitute in and out files
    cc.args << files << "-o" << outname;

    // Substitute additional linker arguments
    cc.args << sanitizedArguments(RipesSettings::value(RIPES_SETTING_LDARGS).toString());

    return cc;
}

CCManager::CCRes CCManager::verifyCC(const QString& CCPath) {
    // Try to set CCPath as current compiler, and compile test program
    m_currentCC = CCPath;

    CCRes res;
    const auto compilerExecInfo = QFileInfo(m_currentCC);

    if (m_currentCC.isEmpty()) {
        res.errorOutput.errMsg = "No path specified";
        res.success = false;
        goto verifyCC_end;
    }

    if (!compilerExecInfo.isExecutable()) {
        res.errorOutput.errMsg = "'" + m_currentCC + "' is not an executable file.";
        res.success = false;
        goto verifyCC_end;
    }

    res = compileRaw(s_testprogram, QString(), false);

    if (!res.success) {
        res.errorOutput._stdout = QString(m_process.readAllStandardOutput());
        res.errorOutput._stderr = QString(m_process.readAllStandardError());
    }

    // Cleanup
    res.clean();

verifyCC_end:
    return res;
}

}  // namespace Ripes
