#include "ccmanager.h"

#include "processorhandler.h"
#include "ripessettings.h"

#include <QProcess>

namespace Ripes {

const static std::vector<QString> s_validAutodetectedCCs = {"riscv64-unknown-elf-gcc"};
const static QString s_testprogram = "int main() { return 0; }";
const static QString s_baseCC = "%1 -march=%2 -mabi=%3 -s %4 -o %5";

CCManager::CCManager() {
    if (RipesSettings::value(RIPES_SETTING_CCPATH) == "") {
        // No previous compiler path has been set. Try to autodetect a valid compiler within the current path
        const auto CCPath = tryAutodetectCC();
        if (!CCPath.isEmpty()) {
            RipesSettings::setValue(RIPES_SETTING_CCPATH, CCPath);
        }
    }

    if (RipesSettings::value(RIPES_SETTING_CCPATH) != "") {
        verifyCC(RipesSettings::value(RIPES_SETTING_CCPATH).toString());
    }
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

void CCManager::verifyCC(const QString& CCPath) {
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

    if (!QFile::exists(testSrcFileOut)) {
#ifdef QT_DEBUG
        qDebug() << "Failed to compile test program";
        qDebug() << "CC output: ";
        qDebug() << "Standard output: " << process.readAllStandardOutput();
        qDebug() << "Standard error: " << process.readAllStandardError();
#endif
    } else {
    }

    // Cleanup
    QFile::remove(testSrcFile.fileName());
    QFile::remove(testSrcFileOut);
}

}  // namespace Ripes
