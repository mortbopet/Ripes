#include "processortab.h"
#include "instructionmodel.h"
#include "pipelinewidget.h"
#include "ui_processortab.h"

#include "parser.h"
#include "pipeline.h"

ProcessorTab::ProcessorTab(QWidget* parent) : QWidget(parent), m_ui(new Ui::ProcessorTab) {
    m_ui->setupUi(this);

    // Setup buttons
    connect(m_ui->start, &QPushButton::toggled, [=](bool state) {
        if (state) {
            m_ui->start->setText("Pause simulation");
            m_timer.start();
        } else {
            m_ui->start->setText("Start simulation");
            m_timer.stop();
        };
        m_ui->step->setEnabled(!state);
    });

    // Setup execution speed slider
    m_ui->execSpeed->setSliderPosition(2);

    connect(m_ui->execSpeed, &QSlider::valueChanged, [=](int pos) {
        // Reverse the slider, going from high to low
        const static int delay = m_ui->execSpeed->maximum() + m_ui->execSpeed->minimum();
        m_timer.setInterval(delay - pos);
    });
    m_ui->execSpeed->valueChanged(m_ui->execSpeed->value());

    connect(m_ui->reset, &QPushButton::clicked, [=] { m_ui->start->setChecked(false); });

    // Setup stepping timer
    connect(&m_timer, &QTimer::timeout, this, &ProcessorTab::on_step_clicked);

    // Setup updating signals
    connect(this, &ProcessorTab::update, m_ui->registerContainer, &RegisterContainerWidget::update);
    connect(this, &ProcessorTab::update, m_ui->pipelineWidget, &PipelineWidget::update);

    // Initially, no file is loaded, disable run, step and reset buttons
    m_ui->reset->setEnabled(false);
    m_ui->step->setEnabled(false);
    m_ui->run->setEnabled(false);
    m_ui->start->setEnabled(false);
}

void ProcessorTab::restart() {
    // Invoked when changes to binary simulation file has been made
    emit update();

    m_ui->step->setEnabled(true);
    m_ui->run->setEnabled(true);
    m_ui->reset->setEnabled(true);
    m_ui->start->setEnabled(true);
}

void ProcessorTab::initRegWidget() {
    // Setup register widget
    m_ui->registerContainer->setRegPtr(Pipeline::getPipeline()->getRegPtr());
    m_ui->registerContainer->init();
}

void ProcessorTab::initInstructionView() {
    // Setup instruction view
    m_instrModel = new InstructionModel(Pipeline::getPipeline()->getStagePCS(),
                                        Pipeline::getPipeline()->getStagePCSPre(), Parser::getParser());
    m_ui->instructionView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->instructionView->setModel(m_instrModel);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    connect(this, &ProcessorTab::update, m_ui->instructionView, QOverload<>::of(&QWidget::update));
    connect(this, &ProcessorTab::update, m_instrModel, &InstructionModel::update);

    // Connect instruction model text changes to the pipeline widget (changing instruction names displayed above each
    // stage)
    connect(m_instrModel, &InstructionModel::textChanged, m_ui->pipelineWidget, &PipelineWidget::stageTextChanged);
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

void ProcessorTab::on_run_clicked() {
    auto pipeline = Pipeline::getPipeline();
    if (pipeline->isReady()) {
        if (pipeline->run()) {
            emit update();
            m_ui->step->setEnabled(false);
            m_ui->start->setEnabled(false);
            m_ui->run->setEnabled(false);
        }
    }
}

void ProcessorTab::on_reset_clicked() {
    Pipeline::getPipeline()->restart();
    emit update();

    m_ui->step->setEnabled(true);
    m_ui->start->setEnabled(true);
    m_ui->run->setEnabled(true);
}

void ProcessorTab::on_step_clicked() {
    auto state = Pipeline::getPipeline()->step();
    emit update();

    if (state) {
        m_ui->step->setEnabled(false);
        m_ui->start->setEnabled(false);
        m_ui->run->setEnabled(false);
    }
}
