#pragma once

#include <QDialog>
#include "ripessettings.h"

namespace Ripes {

namespace Ui {
class SaveDialog;
}

class SaveDialog : public QDialog {
    Q_OBJECT

public:
    explicit SaveDialog(QWidget* parent = nullptr);
    ~SaveDialog();

    static QString getPath() { return RipesSettings::value(RIPES_SETTING_SAVEPATH).toString(); }
    static QString assemblyPath() {
        return RipesSettings::value(RIPES_SETTING_SAVE_ASSEMBLY).toBool() ? getPath() + ".s" : QString();
    }
    static QString binaryPath() {
        return RipesSettings::value(RIPES_SETTING_SAVE_BINARY).toBool() ? getPath() + ".bin" : QString();
    }

    void accept() override;

private:
    void openFileButtonTriggered();
    void pathChanged();

    Ui::SaveDialog* m_ui = nullptr;
};

}  // namespace Ripes
