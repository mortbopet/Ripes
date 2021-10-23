#include "registercontainerwidget.h"
#include "ui_registercontainerwidget.h"

#include "processorhandler.h"
#include "registerwidget.h"

namespace Ripes {

RegisterContainerWidget::RegisterContainerWidget(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::RegisterContainerWidget) {
    m_ui->setupUi(this);
    connect(ProcessorHandler::get(), &ProcessorHandler::procStateChangedNonRun, this,
            &RegisterContainerWidget::updateView);
    connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this, &RegisterContainerWidget::initialize);
    initialize();
}

RegisterContainerWidget::~RegisterContainerWidget() {
    delete m_ui;
}

void RegisterContainerWidget::initialize() {
    m_ui->tabWidget->clear();

    for (const auto& rfid : ProcessorHandler::getProcessor()->registerFiles()) {
        auto registerWidget = new RegisterWidget(rfid, this);
        const unsigned tabIdx = m_ui->tabWidget->count();
        m_ui->tabWidget->insertTab(tabIdx, registerWidget, s_RegsterFileName.at(rfid).shortName);
        m_ui->tabWidget->setTabToolTip(tabIdx, s_RegsterFileName.at(rfid).longName);
        registerWidget->initialize();
    }
    updateView();
}

void RegisterContainerWidget::updateView() {
    for (int i = 0; i < m_ui->tabWidget->count(); ++i) {
        if (auto* regWidget = dynamic_cast<RegisterWidget*>(m_ui->tabWidget->widget(i))) {
            regWidget->updateView();
        }
    }
}

}  // namespace Ripes
