#pragma once

#include <QDialog>
#include "program.h"
#include "ripessettings.h"

namespace Ripes {

namespace Ui {
class SaveDialog;
}

class SaveDialog : public QDialog {
    Q_OBJECT

public:
    explicit SaveDialog(SourceType sourceType, QWidget* parent = nullptr);
    ~SaveDialog();

    QString getPath() { return RipesSettings::value(RIPES_SETTING_SAVEPATH).toString(); }
    QString sourcePath() {
        return RipesSettings::value(RIPES_SETTING_SAVE_SOURCE).toBool()
                   ? getPath() + (sourceType == SourceType::C ? ".c" : ".s")
                   : QString();
    }
    QString binaryPath() {
        return RipesSettings::value(RIPES_SETTING_SAVE_BINARY).toBool() ? getPath() + ".bin" : QString();
    }

    void accept() override;

private:
    void openFileButtonTriggered();
    void pathChanged();

    Ui::SaveDialog* m_ui = nullptr;
    SourceType sourceType;
};

}  // namespace Ripes
