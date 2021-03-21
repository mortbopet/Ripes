#include "ioperipheraltab.h"
#include "ui_ioperipheraltab.h"

#include "editor/csyntaxhighlighter.h"
#include "io/iobase.h"
#include "io/iomanager.h"
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

    if (peripheral->parameters().size() == 0) {
        m_ui->parameterGroupBox->hide();
    } else {
        m_ui->parameterView->setModel(new PeriphParamModel(peripheral, this));
        m_ui->parameterView->horizontalHeader()->setStretchLastSection(true);
        m_ui->parameterView->resizeColumnsToContents();
        m_ui->parameterView->setItemDelegateForColumn(PeriphParamModel::Value, new PeriphParamDelegate(this));
    }

    updateExportsInfo();
}

void IOPeripheralTab::updateExportsInfo() {
    auto symbols = IOManager::get().assemblerSymbolsForPeriph(m_peripheral);
    for (const auto& symbol : symbols) {
        m_ui->exports->appendPlainText("#define " + symbol.first + " " + "(0x" + QString::number(symbol.second, 16) +
                                       ")");
    }
    CSyntaxHighlighter(m_ui->exports->document()).rehighlight();
}

IOPeripheralTab::~IOPeripheralTab() {
    delete m_ui;
}

}  // namespace Ripes
