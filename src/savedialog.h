#pragma once

#include "program.h"
#include "ripessettings.h"
#include <QDialog>
#include <QFileInfo>

namespace Ripes {

namespace Ui {
class SaveDialog;
}

class SaveDialog : public QDialog {
  Q_OBJECT

public:
  explicit SaveDialog(SourceType sourceType, QWidget *parent = nullptr);
  ~SaveDialog();

  QString getPath() {
    return RipesSettings::value(RIPES_SETTING_SAVEPATH).toString();
  }
QString sourcePath() {
    if (!RipesSettings::value(RIPES_SETTING_SAVE_SOURCE).toBool())
        return QString();

    QString path = getPath();
    QFileInfo info(path);

    if (sourceType == SourceType::C) {
        if (info.suffix() != "c")
            path += ".c";
    } else {
        if (info.suffix() != "s")
            path += ".s";
    }

    return path;
}
  QString binaryPath() {
    return RipesSettings::value(RIPES_SETTING_SAVE_BINARY).toBool()
               ? getPath() + ".bin"
               : QString();
  }

  void accept() override;

private:
  void openFileButtonTriggered();
  void pathChanged();

  Ui::SaveDialog *m_ui = nullptr;
  SourceType sourceType;
};

} // namespace Ripes
