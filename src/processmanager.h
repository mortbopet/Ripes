#pragma once

#include <QObject>
#include <QProcess>
#include <QProgressDialog>
#include <QString>

#include "ripessettings.h"
#include "utilities/systemutils.h"

namespace Ripes {

// A process manager is responsible for managing the autodetection and execution of processes. A program manager is a
// singleton for each specialization of the template. This is done through static polymorphism.
template <typename Impl>
class ProcessManager {
public:
    struct ProcessResult {
        QString stdOut;
        QString stdErr;
    };
    ProcessManager() { initialize(); }
    virtual ~ProcessManager() = default;

    static bool hasValidProgram() { return !get().m_programPath.isEmpty(); }
    static const QString& program() { return get().m_programPath; }
    static QString getError() { return get().m_error; }
    static ProcessResult run(const QStringList& args, bool showProgressDialog = false) {
        return get()._run(args, showProgressDialog);
    }

    static Impl& get() {
        static Impl instance;
        return instance;
    }

signals:

    bool trySetProgram(const QString& path) {
        if (static_cast<Impl*>(this)->verifyProgram(path)) {
            m_programPath = path;
        } else {
            m_programPath = QString();
        }
        return !m_programPath.isEmpty();
    }

protected:
    QString m_programPath;
    QProcess m_process;
    bool m_errored = false;
    bool m_aborted = false;

private:
    // Tries to set the program from a set of optional default program names. These program names are expected to be
    // found in the PATH.
    bool tryAutodetectProgram() {
        for (auto defaultPath : static_cast<Impl*>(this)->getDefaultPaths()) {
            if (trySetProgram(defaultPath))
                return true;
        }
        return false;
    }

    // Initializes the program.
    bool initialize() {
        auto settingsPath = static_cast<Impl*>(this)->getSettingsPath();
        if (!settingsPath.isEmpty() && RipesSettings::hasSetting(settingsPath)) {
            // This program has a settings path associated with it. Link up with the settings path, ensuring that we
            // update and revalidate the program on each settings path change.
            auto* observer = RipesSettings::getObserver(settingsPath);
            observer->connect(observer, &SettingObserver::modified,
                              [&](const QVariant& value) { trySetProgram(value.toString()); });

            auto settingsPathValue = RipesSettings::value<QString>(settingsPath);
            if (trySetProgram(settingsPathValue))
                return true;
        }

        // Couldn't initialize from settings, try autodetecting.
        return tryAutodetectProgram();
    }

    // Run the program with a set of arguments.
    ProcessResult _run(const QStringList& args, bool showProgressDialog) {
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
        QProgressDialog progressDiag = QProgressDialog("Executing program...", "Abort", 0, 0, nullptr);
        m_process.connect(&progressDiag, &QProgressDialog::canceled, &m_process, &QProcess::kill);
        m_process.connect(&progressDiag, &QProgressDialog::canceled, &m_process, [this] { m_aborted = true; });
        m_process.connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &progressDiag,
                          &QProgressDialog::reset);
        m_process.connect(&m_process, &QProcess::errorOccurred, &progressDiag, &QProgressDialog::reset);
        m_process.connect(&m_process, &QProcess::errorOccurred, [this]() { m_errored = true; });
        m_process.setWorkingDirectory(static_cast<Impl*>(this)->getWorkingDirectory());
        m_process.setProgram(m_programPath);
        QStringList allArgs = static_cast<Impl*>(this)->getAlwaysArguments();
        allArgs.append(args);
        m_process.setArguments(allArgs);

        m_process.start();
        /** @todo: It is seen that if the process fails upon startup, errorOccurred executes QProgressDialog::reset.
         * However, this call does not prevent the exec() loop from running. Below we check for this case, however this
         * does not remove the race condition. Such race condition seems inherintly tied to how QDialog::exec works and
         * no proper fix has been able to be found (yet).*/
        if (!m_errored && showProgressDialog) {
            progressDiag.exec();
        }
        m_process.waitForFinished();
        return ProcessResult{QString(m_process.readAllStandardOutput()), QString(m_process.readAllStandardError())};
    }
};

}  // namespace Ripes
