#include "processortab.h"
#include "ui_processortab.h"

#include "runner.h"

ProcessorTab::ProcessorTab(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::ProcessorTab) {
    m_ui->setupUi(this);

    connect(m_ui->pipeline, &QRadioButton::toggled, m_ui->forwarding,
            &QCheckBox::setEnabled);
}

void ProcessorTab::initRegWidget(std::vector<uint32_t>* regPtr) {
    // Setup register widget
    m_ui->registerContainer->setRegPtr(regPtr);
    m_ui->registerContainer->init();
}

ProcessorTab::~ProcessorTab() { delete m_ui; }
