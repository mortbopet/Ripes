#include "ccmanager.h"

#include "processorhandler.h"
#include "ripessettings.h"

#include <QProcess>

namespace Ripes {

const static std::vector<QString> s_validAutodetectedCCs = {"riscv64-unknown-elf-gcc", "riscv64-unknown-elf-g++",
                                                            "riscv64-unknown-elf-c++"};
const static QString s_testprogram = "int main() { return 0; }";

/**
 * @brief s_baseCC
 * Base compiler command.
 * - %1: path to compiler executable
 * - %2: machine architecture
 * - %3: machine ABI
 * - -x c: Enforce compilation as C language (allows us to use C++ compilers)
 * - %4: input source file
 * - %5: output executable
 */
const static QString s_baseCC = "%1 -march=%2 -mabi=%3 -x c -s %4 -o %5";

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

bool CCManager::hasValidCC() const {
    return !m_currentCC.isEmpty();
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
    const bool success = verifyCC(CC);
    if (success) {
        m_currentCC = CC;
    } else {
        m_currentCC = QString();
    }
    emit ccChanged(success);
    return success;
}

bool CCManager::verifyCC(const QString& CCPath) {
    // Write test program to temporary file with a .c extension

    QTemporaryFile testSrcFile(QDir::tempPath() + QDir::separator() + QCoreApplication::applicationName() +
                               ".XXXXXX.c");
    if (testSrcFile.open()) {
        QTextStream stream(&testSrcFile);
        stream << s_testprogram;
    }
    Q_ASSERT(!testSrcFile.fileName().isEmpty());
    const QString& testSrcFileOut = testSrcFile.fileName() + ".out";

    const auto& currentISA = ProcessorHandler::get()->currentISA();

    // Generate compile command
    QString s_cc = s_baseCC;

    // Substitute compiler path
    s_cc = s_cc.arg(CCPath);

    // Substitute machine architecture
    s_cc = s_cc.arg(currentISA->CCmarch());

    // Substitute machine ABI
    s_cc = s_cc.arg(currentISA->CCmabi());

    // Substitute in and out files
    s_cc = s_cc.arg(testSrcFile.fileName()).arg(testSrcFileOut);

    // Run compiler
    QProcess process;
    process.start(s_cc);
    process.waitForFinished();

    const bool success = QFile::exists(testSrcFileOut);

#ifdef QT_DEBUG
    if (!success) {
        qDebug() << "Failed to compile test program";
        qDebug() << "CC output: ";
        qDebug() << "Standard output: " << process.readAllStandardOutput();
        qDebug() << "Standard error: " << process.readAllStandardError();
    }
#endif

    // Cleanup
    QFile::remove(testSrcFile.fileName());
    QFile::remove(testSrcFileOut);

    return success;
}

}  // namespace Ripes
