#include "processortab.h"
#include "ui_processortab.h"

#include "runner.h"

ProcessorTab::ProcessorTab(QWidget* parent) : QWidget(parent), m_ui(new Ui::ProcessorTab) {
    m_ui->setupUi(this);

    // Setup buttons
    connect(m_ui->start, &QPushButton::toggled, [=](bool state) {
        if (state) {
            m_ui->start->setText("Pause simulation");
        } else {
            m_ui->start->setText("Start simulation");
        };
        m_ui->step->setEnabled(!state);
    });

    connect(m_ui->reset, &QPushButton::clicked, [=] { m_ui->start->setChecked(false); });
}

void ProcessorTab::initRegWidget(std::vector<uint32_t>* regPtr) {
    // Setup register widget
    m_ui->registerContainer->setRegPtr(regPtr);
    m_ui->registerContainer->init();
}

ProcessorTab::~ProcessorTab() {
    delete m_ui;
}

void ProcessorTab::on_toolButton_clicked() {
    m_ui->pipelineWidget->expandToView();
}
