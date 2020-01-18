#include "processortab.h"
#include "ui_processortab.h"

#include <QScrollBar>
#include <QSpinBox>

#include "instructionmodel.h"
#include "parser.h"
#include "processorhandler.h"
#include "processorregistry.h"
#include "processorselectiondialog.h"
#include "registermodel.h"
#include "stagetablemodel.h"
#include "stagetablewidget.h"

#include "VSRTL/graphics/vsrtl_widget.h"

#include "processors/ripesprocessor.h"

namespace Ripes {

ProcessorTab::ProcessorTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::ProcessorTab) {
    m_ui->setupUi(this);

    m_vsrtlWidget = m_ui->vsrtlWidget;

    // Load the default processor
    ProcessorHandler::get()->loadProcessorToWidget(m_vsrtlWidget);

    m_stageModel = new StageTableModel(this);
    connect(this, &ProcessorTab::update, m_stageModel, &StageTableModel::processorWasClocked);

    updateInstructionModel();
    m_ui->registerWidget->updateModel();
    connect(this, &ProcessorTab::update, m_ui->registerWidget, &RegisterWidget::updateView);

    setupSimulatorActions();

    // Connect ECALL functionality to application output log and scroll to bottom
    connect(this, &ProcessorTab::appendToLog, [this](QString string) {
        m_ui->console->insertPlainText(string);
        m_ui->console->verticalScrollBar()->setValue(m_ui->console->verticalScrollBar()->maximum());
    });

    // Make processor view stretch wrt. consoles
    m_ui->pipelinesplitter->setStretchFactor(0, 1);
    m_ui->pipelinesplitter->setStretchFactor(1, 0);

    // Make processor view stretch wrt. right side tabs
    m_ui->viewSplitter->setStretchFactor(0, 1);
    m_ui->viewSplitter->setStretchFactor(1, 0);

    m_ui->consolesTab->removeTab(1);

    // Initially, no file is loaded, disable toolbuttons
    enableSimulatorControls();
}

void ProcessorTab::printToLog(const QString& text) {
    m_ui->console->insertPlainText(text);
    m_ui->console->verticalScrollBar()->setValue(m_ui->console->verticalScrollBar()->maximum());
}

void ProcessorTab::setupSimulatorActions() {
    const QIcon processorIcon = QIcon(":/icons/cpu.svg");
    m_selectProcessorAction = new QAction(processorIcon, "Select processor", this);
    connect(m_selectProcessorAction, &QAction::triggered, this, &ProcessorTab::processorSelection);
    m_toolbar->addAction(m_selectProcessorAction);
    m_toolbar->addSeparator();

    const QIcon resetIcon = QIcon(":/icons/reset.svg");
    m_resetAction = new QAction(resetIcon, "Reset (F3)", this);
    connect(m_resetAction, &QAction::triggered, this, &ProcessorTab::reset);
    m_resetAction->setShortcut(QKeySequence("F3"));
    m_toolbar->addAction(m_resetAction);

    const QIcon reverseIcon = QIcon(":/icons/rewind.svg");
    m_reverseAction = new QAction(reverseIcon, "Rewind (F4)", this);
    connect(m_reverseAction, &QAction::triggered, this, &ProcessorTab::rewind);
    m_reverseAction->setShortcut(QKeySequence("F4"));
    m_toolbar->addAction(m_reverseAction);

    const QIcon clockIcon = QIcon(":/icons/step.svg");
    m_clockAction = new QAction(clockIcon, "Clock (F5)", this);
    connect(m_clockAction, &QAction::triggered, this, &ProcessorTab::clock);
    m_clockAction->setShortcut(QKeySequence("F5"));
    m_toolbar->addAction(m_clockAction);

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &ProcessorTab::clock);

    const QIcon startAutoClockIcon = QIcon(":/icons/step-clock.svg");
    const QIcon stopAutoTimerIcon = QIcon(":/icons/stop-clock.svg");
    m_autoClockAction = new QAction(startAutoClockIcon, "Auto clock (F6)", this);
    m_autoClockAction->setShortcut(QKeySequence("F6"));
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
    m_runAction->setCheckable(true);
    m_runAction->setChecked(false);
    connect(m_runAction, &QAction::toggled, this, &ProcessorTab::run);
    m_toolbar->addAction(m_runAction);
    m_toolbar->addSeparator();

    const QIcon tagIcon = QIcon(":/icons/tag.svg");
    m_displayValuesAction = new QAction(tagIcon, "Display signal values", this);
    m_displayValuesAction->setCheckable(true);
    m_displayValuesAction->setChecked(false);
    connect(m_displayValuesAction, &QAction::triggered, m_vsrtlWidget, &vsrtl::VSRTLWidget::setOutputPortValuesVisible);
    m_toolbar->addAction(m_displayValuesAction);

    const QIcon expandIcon = QIcon(":/icons/expand.svg");
    m_fitViewAction = new QAction(expandIcon, "Fit to view", this);
    // connect(m_fitViewAction, &QAction::triggered, this, &ProcessorTab::expandView);
    m_toolbar->addAction(m_fitViewAction);

    const QIcon tableIcon = QIcon(":/icons/spreadsheet.svg");
    m_pipelineTableAction = new QAction(tableIcon, "Show pipelining table", this);
    connect(m_pipelineTableAction, &QAction::triggered, this, &ProcessorTab::showStageTable);
    m_toolbar->addAction(m_pipelineTableAction);
}

void ProcessorTab::pause() {
    m_autoClockAction->setChecked(false);
    m_runAction->setChecked(false);
    m_reverseAction->setEnabled(m_vsrtlWidget->isRewindable());
}

void ProcessorTab::processorSelection() {
    ProcessorSelectionDialog diag;
    if (diag.exec()) {
        // New processor model was selected
        m_vsrtlWidget->clearDesign();
        ProcessorHandler::get()->selectProcessor(
            diag.selectedID, ProcessorRegistry::getDescription(diag.selectedID).defaultRegisterVals);
        ProcessorHandler::get()->loadProcessorToWidget(m_vsrtlWidget);
        updateInstructionModel();
        m_ui->registerWidget->updateModel();
        update();
    }
}

void ProcessorTab::updateInstructionModel() {
    auto* oldModel = m_instrModel;
    m_instrModel = new InstructionModel(this);

    // Update the instruction view according to the newly created model
    m_ui->instructionView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->instructionView->setModel(m_instrModel);

    // Only the instruction column should stretch
    m_ui->instructionView->horizontalHeader()->setMinimumSectionSize(1);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(InstructionModel::Breakpoint,
                                                                    QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(InstructionModel::PC,
                                                                    QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(InstructionModel::Stage,
                                                                    QHeaderView::ResizeToContents);
    m_ui->instructionView->horizontalHeader()->setSectionResizeMode(InstructionModel::Instruction,
                                                                    QHeaderView::Stretch);

    connect(this, &ProcessorTab::update, m_instrModel, &InstructionModel::processorWasClocked);

    // Make the instruction view follow the instruction which is currently present in the first stage of the processor
    connect(m_instrModel, &InstructionModel::firstStageInstrChanged, this, &ProcessorTab::setInstructionViewCenterAddr);

    if (oldModel) {
        delete oldModel;
    }
}

void ProcessorTab::restart() {
    // Invoked when changes to binary simulation file has been made
    emit update();
    enableSimulatorControls();
}

ProcessorTab::~ProcessorTab() {
    delete m_ui;
}

void ProcessorTab::processorFinished() {
    // Disallow further clocking of the circuit
    m_clockAction->setEnabled(false);
    m_autoClockAction->setChecked(false);
    m_autoClockAction->setEnabled(false);
    m_runAction->setEnabled(false);
    m_runAction->setChecked(false);
}

void ProcessorTab::enableSimulatorControls() {
    m_clockAction->setEnabled(true);
    m_autoClockAction->setEnabled(true);
    m_runAction->setEnabled(true);
    m_reverseAction->setEnabled(m_vsrtlWidget->isRewindable());
    m_resetAction->setEnabled(true);
}

void ProcessorTab::reset() {
    m_autoClockAction->setChecked(false);
    m_vsrtlWidget->reset();
    m_stageModel->reset();
    emit update();

    enableSimulatorControls();
    emit appendToLog("\n");
}

void ProcessorTab::setInstructionViewCenterAddr(uint32_t address) {
    const auto index = addressToIndex(address);
    const auto view = m_ui->instructionView;
    const auto rect = view->rect();
    int indexTop = view->indexAt(rect.topLeft()).row();
    int indexBot = view->indexAt(rect.bottomLeft()).row();
    indexBot = indexBot < 0 ? m_instrModel->rowCount() : indexBot;

    const int nItemsVisible = indexBot - indexTop;

    // move scrollbar if if is not visible
    if (index <= indexTop || index >= indexBot) {
        auto scrollbar = view->verticalScrollBar();
        scrollbar->setValue(index - nItemsVisible / 2);
    }
}

void ProcessorTab::runFinished() {
    pause();
    ProcessorHandler::get()->checkProcessorFinished();
    emit update();
}

void ProcessorTab::run(bool state) {
    if (state) {
        ProcessorHandler::get()->run();
    } else {
        ProcessorHandler::get()->stop();
    }
}

void ProcessorTab::rewind() {
    m_vsrtlWidget->rewind();
    enableSimulatorControls();
    emit update();
}

void ProcessorTab::clock() {
    m_vsrtlWidget->clock();
    ProcessorHandler::get()->checkValidExecutionRange();
    if (ProcessorHandler::get()->checkBreakpoint()) {
        pause();
    }
    ProcessorHandler::get()->checkProcessorFinished();
    m_reverseAction->setEnabled(m_vsrtlWidget->isRewindable());

    emit update();
}

void ProcessorTab::showStageTable() {
    auto w = StageTableWidget(m_stageModel, this);
    w.exec();
}
}  // namespace Ripes
