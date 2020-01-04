#include "processortab.h"
#include "ui_processortab.h"

#include <QFileDialog>
#include <QScrollBar>
#include <QSpinBox>

#include "graphics/pipelinewidget.h"
#include "instructionmodel.h"
#include "parser.h"
#include "pipeline.h"
#include "rundialog.h"

ProcessorTab::ProcessorTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::ProcessorTab) {
    m_ui->setupUi(this);

    setupSimulatorActions();

    // Setup updating signals
    connect(this, &ProcessorTab::update, m_ui->registerContainer, &RegisterContainerWidget::update);
    connect(this, &ProcessorTab::update, m_ui->pipelineWidget, &PipelineWidget::update);
    connect(this, &ProcessorTab::update, this, &ProcessorTab::updateMetrics);

    // Connect ECALL functionality to application output log and scroll to bottom
    connect(this, &ProcessorTab::appendToLog, [this](QString string) {
        m_ui->console->insertPlainText(string);
        m_ui->console->verticalScrollBar()->setValue(m_ui->console->verticalScrollBar()->maximum());
    });

    // Setup splitter such that consoles are always as small as possible
    m_ui->pipelinesplitter->setStretchFactor(0, 2);

    const auto splitterSize = m_ui->pipelinesplitter->size();
    m_ui->pipelinesplitter->setSizes(QList<int>() << splitterSize.height() - (m_ui->consolesTab->minimumHeight() - 1)
                                                  << (m_ui->consolesTab->minimumHeight() + 1));
    m_ui->consolesTab->removeTab(1);

    // Initially, no file is loaded, disable toolbuttons
    updateActionState();
}

void ProcessorTab::setupSimulatorActions() {
    const QIcon resetIcon = QIcon(":/icons/reset.svg");
    m_resetAction = new QAction(resetIcon, "Reset (F4)", this);
    connect(m_resetAction, &QAction::triggered, this, &ProcessorTab::reset);
    m_resetAction->setShortcut(QKeySequence("F4"));
    m_toolbar->addAction(m_resetAction);

    const QIcon reverseIcon = QIcon(":/icons/rewind.svg");
    m_reverseAction = new QAction(reverseIcon, "Rewind (F5)", this);
    m_reverseAction->setShortcut(QKeySequence("F5"));
    m_toolbar->addAction(m_reverseAction);

    const QIcon clockIcon = QIcon(":/icons/step.svg");
    m_clockAction = new QAction(clockIcon, "Clock (F6)", this);
    connect(m_clockAction, &QAction::triggered, this, &ProcessorTab::clock);
    m_clockAction->setShortcut(QKeySequence("F6"));
    m_toolbar->addAction(m_clockAction);

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &ProcessorTab::clock);

    const QIcon startAutoClockIcon = QIcon(":/icons/step-clock.svg");
    const QIcon stopAutoTimerIcon = QIcon(":/icons/stop-clock.svg");
    m_autoClockAction = new QAction(startAutoClockIcon, "Auto clock (F7)", this);
    m_autoClockAction->setShortcut(QKeySequence("F7"));
    m_autoClockAction->setCheckable(true);
    connect(m_autoClockAction, &QAction::toggled, [=](bool checked) {
        if (!checked) {
            timer->stop();
            m_autoClockAction->setIcon(startAutoClockIcon);
        } else {
            timer->start();
            m_autoClockAction->setIcon(stopAutoTimerIcon);
        }
    });
    m_autoClockAction->setChecked(false);
    m_toolbar->addAction(m_autoClockAction);

    m_autoClockInterval = new QSpinBox(this);
    m_autoClockInterval->setRange(1, 10000);
    m_autoClockInterval->setSuffix(" ms");
    m_autoClockInterval->setToolTip("Auto clock interval");
    connect(m_autoClockInterval, qOverload<int>(&QSpinBox::valueChanged),
            [timer](int msec) { timer->setInterval(msec); });
    m_autoClockInterval->setValue(100);
    m_toolbar->addWidget(m_autoClockInterval);

    const QIcon runIcon = QIcon(":/icons/run.svg");
    m_runAction = new QAction(runIcon, "Run (F8)", this);
    m_runAction->setShortcut(QKeySequence("F8"));
    connect(m_runAction, &QAction::triggered, this, &ProcessorTab::run);
    m_toolbar->addAction(m_runAction);

    m_toolbar->addSeparator();

    const QIcon tagIcon = QIcon(":/icons/tag.svg");
    m_displayValuesAction = new QAction(tagIcon, "Display signal values", this);
    m_displayValuesAction->setCheckable(true);
    m_displayValuesAction->setChecked(false);
    connect(m_displayValuesAction, &QAction::triggered, this, &ProcessorTab::displayValues);
    m_toolbar->addAction(m_displayValuesAction);

    const QIcon expandIcon = QIcon(":/icons/expand.svg");
    m_fitViewAction = new QAction(expandIcon, "Fit to view", this);
    connect(m_fitViewAction, &QAction::triggered, this, &ProcessorTab::expandView);
    m_toolbar->addAction(m_fitViewAction);

    const QIcon tableIcon = QIcon(":/icons/spreadsheet.svg");
    m_pipelineTableAction = new QAction(tableIcon, "Show pipelining table", this);
    connect(m_pipelineTableAction, &QAction::triggered, this, &ProcessorTab::showPipeliningTable);
    m_toolbar->addAction(m_pipelineTableAction);
}

void ProcessorTab::restart() {
    // Invoked when changes to binary simulation file has been made
    emit update();
    updateActionState();
}

void ProcessorTab::initRegWidget() {
    // Setup register widget
    m_ui->registerContainer->setRegPtr(Pipeline::getPipeline()->getRegPtr());
    m_ui->registerContainer->init();
}

void ProcessorTab::updateMetrics() {
    m_ui->cycleCount->setText(QString::number(Pipeline::getPipeline()->getCycleCount()));
    m_ui->nInstrExecuted->setText(QString::number(Pipeline::getPipeline()->getInstructionsExecuted()));
}

void ProcessorTab::initInstructionView() {
    // Setup instruction view
    m_instrModel = new InstructionModel(Pipeline::getPipeline()->getStagePCS(),
                                        Pipeline::getPipeline()->getStagePCSPre(), Parser::getParser());
    m_ui->instructionView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->instructionView->setModel(m_instrModel);
    m_ui->instructionView->horizontalHeader()->setMinimumSectionSize(1);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    connect(this, &ProcessorTab::update, m_ui->instructionView, QOverload<>::of(&QWidget::update));
    connect(this, &ProcessorTab::update, m_instrModel, &InstructionModel::update);
    connect(m_instrModel, &InstructionModel::currentIFRow, this, &ProcessorTab::setCurrentInstruction);
    // Connect instruction model text changes to the pipeline widget (changing instruction names displayed above each
    // stage)
    connect(m_instrModel, &InstructionModel::textChanged, m_ui->pipelineWidget, &PipelineWidget::stageTextChanged);
}

ProcessorTab::~ProcessorTab() {
    delete m_ui;
}

void ProcessorTab::expandView() {
    m_ui->pipelineWidget->expandToView();
}

void ProcessorTab::displayValues(bool checked) {
    m_ui->pipelineWidget->displayAllValues(checked);
}

void ProcessorTab::run() {
    m_autoClockAction->setChecked(false);
    auto pipeline = Pipeline::getPipeline();
    RunDialog dialog(this);
    if (pipeline->isReady()) {
        if (dialog.exec() && pipeline->isFinished()) {
            emit update();
            updateActionState();
        }
    }
}

void ProcessorTab::updateActionState() {
    const auto ready = Pipeline::getPipeline()->isReady();

    m_clockAction->setEnabled(ready);
    m_autoClockAction->setEnabled(ready);
    m_runAction->setEnabled(ready);
    m_displayValuesAction->setEnabled(ready);
    m_fitViewAction->setEnabled(ready);
    m_pipelineTableAction->setEnabled(ready);
    m_reverseAction->setEnabled(ready);
    m_resetAction->setEnabled(ready);

    if (Pipeline::getPipeline()->isFinished()) {
        m_clockAction->setEnabled(false);
        m_autoClockAction->setChecked(false);
        m_autoClockAction->setEnabled(false);
        m_runAction->setEnabled(false);
    }
}

void ProcessorTab::reset() {
    m_autoClockAction->setChecked(false);
    Pipeline::getPipeline()->restart();
    emit update();

    updateActionState();
    emit appendToLog("\n");
}

void ProcessorTab::setCurrentInstruction(int row) {
    // model emits signal with current IF instruction row
    auto instructionView = m_ui->instructionView;
    auto rect = instructionView->rect();
    int indexTop = instructionView->indexAt(rect.topLeft()).row();
    int indexBot = instructionView->indexAt(rect.bottomLeft()).row();

    int nItems = indexBot - indexTop;

    // move scrollbar if if is not visible
    if (row <= indexTop || row >= indexBot) {
        auto scrollbar = m_ui->instructionView->verticalScrollBar();
        scrollbar->setValue(row - nItems / 2);
    }
}

void ProcessorTab::clock() {
    auto pipeline = Pipeline::getPipeline();
    auto state = pipeline->step();

    const auto ecallVal = pipeline->checkEcall(true);
    if (ecallVal.first != Pipeline::ECALL::none) {
        handleEcall(ecallVal);
    }

    emit update();

    // Pipeline has finished executing
    if (pipeline->isFinished() || (state == 1 && ecallVal.first == Pipeline::ECALL::exit)) {
        updateActionState();
    }
}

bool ProcessorTab::handleEcall(const std::pair<Pipeline::ECALL, int32_t>& ecall_val) {
    // Check if ecall has been invoked
    switch (ecall_val.first) {
        case Pipeline::ECALL::none:
            break;
        case Pipeline::ECALL::print_string: {
            emit appendToLog(Parser::getParser()->getStringAt(static_cast<uint32_t>(ecall_val.second)));
            break;
        }
        case Pipeline::ECALL::print_int: {
            emit appendToLog(QString::number(ecall_val.second));
            break;
        }
        case Pipeline::ECALL::print_char: {
            emit appendToLog(QChar(ecall_val.second));
            break;
        }
        case Pipeline::ECALL::exit: {
            return true;  // The simulator will now take a few cycles to stop
        }
    }

    return true;  // continue
}

void ProcessorTab::showPipeliningTable() {
    // Setup pipeline table window
    PipelineTable window;
    PipelineTableModel model;
    window.setModel(&model);
    window.exec();
}
