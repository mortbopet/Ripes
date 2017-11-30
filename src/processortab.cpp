#include "processortab.h"
#include "instructionmodel.h"
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

void ProcessorTab::initRegWidget() {
    // Setup register widget
    m_ui->registerContainer->setRegPtr(Runner::getRunner()->getRegPtr());
    m_ui->registerContainer->init();
}

void ProcessorTab::initInstructionView() {
    // Setup instruction view
    m_instrModel = new InstructionModel(Runner::getRunner()->getStagePCS(), Parser::getParser());
    m_ui->instructionView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_instrModel->setMemory(Runner::getRunner()->getMemoryPtr(), Runner::getRunner()->getTextSize());
    m_ui->instructionView->setModel(m_instrModel);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
}

ProcessorTab::~ProcessorTab() {
    delete m_ui;
}

void ProcessorTab::on_expandView_clicked() {
    m_ui->pipelineWidget->expandToView();
}

void ProcessorTab::on_displayValues_toggled(bool checked) {
    m_ui->pipelineWidget->displayAllValues(checked);
}
