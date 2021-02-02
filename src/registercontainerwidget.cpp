#include "registercontainerwidget.h"
#include "ui_registercontainerwidget.h"

#include "processorhandler.h"
#include "registerwidget.h"

namespace Ripes {

RegisterContainerWidget::RegisterContainerWidget(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::RegisterContainerWidget) {
    m_ui->setupUi(this);
}

RegisterContainerWidget::~RegisterContainerWidget() {
    delete m_ui;
}

void RegisterContainerWidget::initialize() {
    m_ui->tabWidget->clear();

    for (const auto& rfid : ProcessorHandler::get()->getProcessor()->registerFiles()) {
        auto registerWidget = new RegisterWidget(rfid, this);
        m_ui->tabWidget->insertTab(m_ui->tabWidget->count(), registerWidget, s_RegsterFileName.at(rfid));
        registerWidget->initialize();
    }
}

void RegisterContainerWidget::updateView() {
    for (int i = 0; i < m_ui->tabWidget->count(); i++) {
        if (auto* regWidget = dynamic_cast<RegisterWidget*>(m_ui->tabWidget->widget(i))) {
            regWidget->updateView();
        }
    }
}

}  // namespace Ripes
