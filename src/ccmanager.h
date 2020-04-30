#pragma once

#include <QFile>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTextDocument>

namespace Ripes {

/**
 * @brief The CCManager class
 * Manages the detection, verification and execution of a valid C/C++ compiler suitable for the ISAs targetted by the
 * various processor models of Ripes.
 */
class CCManager : public QObject {
    Q_OBJECT
public:
    struct CCRes {
        QString inFile;
        QString outFile;
        bool success;

        void clean() {
            QFile::remove(inFile);
            QFile::remove(outFile);
        }
    };

    static CCManager& get() {
        static CCManager manager;
        return manager;
    }

    static bool hasValidCC();
    static const QString& currentCC() { return get().m_currentCC; }

    static QString getError();

    /**
     * @brief compile
     * Runs the current compiler, with the current set of arguments, on file @p filename. @returns information about the
     * compiled file, such as source file, output file and compilation status. If no @p outname has been provided, the
     * output file will be placed in a temporary directory.
     */
    CCRes compile(const QString& filename, QString outname = QString());
    CCRes compile(const QTextDocument* source, QString outname = QString());
    CCRes compileRaw(const QString& rawsource, QString outname = QString());

signals:
    /**
     * @brief ccChanged
     * Emitted whenever the current CC path changed. @param valid indicates whether the CC was successfully validated.
     */
    void ccChanged(bool valid);

public slots:
    /**
     * @brief trySetCC
     * Attempts to set the path @param CC to be the current compiler.
     * @return true if the CC was successfully validated and set
     */
    bool trySetCC(const QString& CC);

private:
    QString createCompileCommand(const QString& filename, const QString& outname) const;

    /**
     * @brief tryAutodetectCC
     * Will attempt to scan the current PATH to locate a valid compiler
     */
    QString tryAutodetectCC();

    /**
     * @brief verifyCC
     * Attempts to compile a simple test program using the provided compiler path @param CC.
     * @returns true if successful.
     */
    bool verifyCC(const QString& CC);

    CCManager();
    QString m_currentCC;
    QProcess m_process;
};

}  // namespace Ripes
