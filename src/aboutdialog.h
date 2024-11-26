#pragma once

#include "ui_aboutdialog.h"
#include <QDialog>
#include <qwidget.h>

namespace Ripes {

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog {
  Q_OBJECT
public:
  explicit AboutDialog(QWidget *parent = nullptr);
  ~AboutDialog() override;

private:
  Ui::AboutDialog *m_ui = nullptr;
};

} // namespace Ripes
