#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "version.h"

namespace Ripes {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), m_ui(new Ui::AboutDialog) {
  m_ui->setupUi(this);
  setWindowTitle("About Ripes");
}

AboutDialog::~AboutDialog() { delete m_ui; }

} // namespace Ripes
