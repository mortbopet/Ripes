#include "ioperipheraltab.h"
#include "ui_ioperipheraltab.h"

#include "io/iobase.h"
#include "io/periphparammodel.h"
#include "io/registermapmodel.h"

namespace Ripes {

IOPeripheralTab::IOPeripheralTab(QWidget* parent, IOBase* peripheral)
    : QWidget(parent), m_peripheral(peripheral), m_ui(new Ui::IOPeripheralTab) {
    m_ui->setupUi(this);

    m_ui->description->setText(m_peripheral->description());
    m_ui->registerMapView->setModel(new RegisterMapModel(peripheral, this));
    m_ui->registerMapView->horizontalHeader()->setStretchLastSection(true);
    m_ui->registerMapView->resizeColumnsToContents();

    m_ui->parameterView->setModel(new PeriphParamModel(peripheral, this));
    m_ui->parameterView->horizontalHeader()->setStretchLastSection(true);
    m_ui->parameterView->resizeColumnsToContents();
}

IOPeripheralTab::~IOPeripheralTab() {
    delete m_ui;
}

}  // namespace Ripes
