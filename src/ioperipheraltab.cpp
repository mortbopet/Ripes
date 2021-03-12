#include "ioperipheraltab.h"
#include "ui_ioperipheraltab.h"

#include "io/iobase.h"

namespace Ripes {

IOPeripheralTab::IOPeripheralTab(QWidget* parent, IOBase* peripheral)
    : QWidget(parent), m_peripheral(peripheral), m_ui(new Ui::IOPeripheralTab) {
    m_ui->setupUi(this);

    m_ui->description->setText(m_peripheral->description());
}

IOPeripheralTab::~IOPeripheralTab() {
    delete m_ui;
}

}  // namespace Ripes
