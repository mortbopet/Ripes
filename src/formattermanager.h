#pragma once

#include <QCoreApplication>

#include "processmanager.h"
#include "ripessettings.h"

namespace Ripes {

class FormatterManager : public ProcessManager<FormatterManager> {
    friend class ProcessManager<FormatterManager>;

private:
    bool verifyProgram(const QString& path) { return true; }
    QString getSettingsPath() { return RIPES_SETTING_FORMATTER_PATH; }
    QStringList getAlwaysArguments() {
        return RipesSettings::value(RIPES_SETTING_FORMATTER_ARGS).toString().split(" ");
    }
    QStringList getDefaultPaths() { return {"clang-format"}; }
    QString getWorkingDirectory() { return QCoreApplication::applicationDirPath(); }
};

}  // namespace Ripes
