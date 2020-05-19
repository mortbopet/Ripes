#include "processortab.h"
#include "ui_processortab.h"

#include <QDir>
#include <QMessageBox>
#include <QScrollBar>
#include <QSpinBox>
#include <QTemporaryFile>

#include "instructionmodel.h"
#include "parser.h"
#include "processorhandler.h"
#include "processorregistry.h"
#include "processorselectiondialog.h"
#include "registermodel.h"
#include "ripessettings.h"
#include "stagetablemodel.h"
#include "stagetablewidget.h"

#include "VSRTL/graphics/vsrtl_widget.h"

#include "processors/ripesprocessor.h"

namespace Ripes {

namespace {
//
inline QString convertToSIUnits(const double l_value, int precision = 2) {
    QString unit;
    double value;

    if (l_value < 0) {
        value = l_value * -1;
    } else {
        value = l_value;
    }

    if (value >= 1000000 && value < 1000000000) {
        value = value / 1000000;
        unit = "M";
    } else if (value >= 1000 && value < 1000000) {
        value = value / 1000;
        unit = "K";
    } else if (value >= 1 && value < 1000) {
        value = value * 1;
    } else if ((value * 1000) >= 1 && value < 1000) {
        value = value * 1000;
        unit = "m";
    } else if ((value * 1000000) >= 1 && value < 1000000) {
        value = value * 1000000;
        unit = QChar(0x00B5);
    } else if ((value * 1000000000) >= 1 && value < 1000000000) {
        value = value * 1000000000;
        unit = "n";
    }

    if (l_value > 0) {
        return (QString::number(value, 10, precision) + " " + unit);
    } else if (l_value < 0) {
        return (QString::number(value * -1, 10, precision) + " " + unit);
    }
    return QString::number(0) + " ";
}
}  // namespace

ProcessorTab::ProcessorTab(QToolBar* controlToolbar, QToolBar* additionalToolbar, QWidget* parent)
    : RipesTab(additionalToolbar, parent), m_ui(new Ui::ProcessorTab) {
    m_ui->setupUi(this);

    m_vsrtlWidget = m_ui->vsrtlWidget;

    // Load the default constructed processor to the VSRTL widget
    loadProcessorToWidget(ProcessorRegistry::getDescription(ProcessorHandler::get()->getID()).layouts.at(0));

    // By default, lock the VSRTL widget
    m_vsrtlWidget->setLocked(true);

    m_stageModel = new StageTableModel(this);
    connect(this, &ProcessorTab::update, m_stageModel, &StageTableModel::processorWasClocked);

    updateInstructionModel();
    m_ui->registerWidget->updateModel();
    connect(this, &ProcessorTab::update, m_ui->registerWidget, &RegisterWidget::updateView);
    connect(this, &ProcessorTab::update, this, &ProcessorTab::updateStatistics);
    connect(this, &ProcessorTab::update, this, &ProcessorTab::updateInstructionLabels);

    setupSimulatorActions(controlToolbar);

    // Setup statistics update timer
    m_statUpdateTimer = new QTimer(this);
    m_statUpdateTimer->setInterval(100);
    connect(m_statUpdateTimer, &QTimer::timeout, this, &ProcessorTab::updateStatistics);

    // Connect ECALL functionality to application output log and scroll to bottom
    connect(this, &ProcessorTab::appendToLog, [this](QString string) {
        m_ui->console->insertPlainText(string);
        m_ui->console->verticalScrollBar()->setValue(m_ui->console->verticalScrollBar()->maximum());
    });

    // Connect changes in VSRTL reversible stack size to checking whether the simulator is reversible
    connect(RipesSettings::getObserver(RIPES_SETTING_REWINDSTACKSIZE), &SettingObserver::modified,
            [=](const auto& size) { m_reverseAction->setEnabled(m_vsrtlWidget->isReversible()); });

    // Make processor view stretch wrt. consoles
    m_ui->pipelinesplitter->setStretchFactor(0, 1);
    m_ui->pipelinesplitter->setStretchFactor(1, 0);

    // Make processor view stretch wrt. right side tabs
    m_ui->viewSplitter->setStretchFactor(0, 1);
    m_ui->viewSplitter->setStretchFactor(1, 0);

    // Initially, no file is loaded, disable toolbuttons
    enableSimulatorControls();
}

void ProcessorTab::printToLog(const QString& text) {
    m_ui->console->moveCursor(QTextCursor::End);
    m_ui->console->insertPlainText(text);
    m_ui->console->verticalScrollBar()->setValue(m_ui->console->verticalScrollBar()->maximum());
}

void ProcessorTab::loadLayout(const Layout& layout) {
    if (layout.name.isEmpty() || layout.file.isEmpty())
        return;  // Not a valid layout

    if (layout.stageLabelPositions.size() != ProcessorHandler::get()->getProcessor()->stageCount()) {
        Q_ASSERT(false && "A stage label position must be specified for each stage");
    }

    // cereal expects the archive file to be present standalone on disk, and available through an ifstream. Copy the
    // resource layout file (bundled within the binary as a Qt resource) to a temporary file, for loading the layout.
    const auto& layoutResourceFilename = layout.file;
    QFile layoutResourceFile(layoutResourceFilename);
    QTemporaryFile* tmpLayoutFile = QTemporaryFile::createNativeFile(layoutResourceFile);
    if (!tmpLayoutFile->open()) {
        QMessageBox::warning(this, "Error", "Could not create temporary layout file");
        return;
    }

    m_vsrtlWidget->getTopLevelComponent()->loadLayoutFile(tmpLayoutFile->fileName());
    tmpLayoutFile->remove();

    // Adjust stage label positions
    const auto& parent = m_stageInstructionLabels.at(0)->parentItem();
    for (unsigned i = 0; i < m_stageInstructionLabels.size(); i++) {
        auto& label = m_stageInstructionLabels.at(i);
        label->setPos(parent->boundingRect().width() * layout.stageLabelPositions.at(i), 0);
    }
}

void ProcessorTab::setupSimulatorActions(QToolBar* controlToolbar) {
    const QIcon processorIcon = QIcon(":/icons/cpu.svg");
    m_selectProcessorAction = new QAction(processorIcon, "Select processor", this);
    connect(m_selectProcessorAction, &QAction::triggered, this, &ProcessorTab::processorSelection);
    controlToolbar->addAction(m_selectProcessorAction);
    controlToolbar->addSeparator();

    const QIcon resetIcon = QIcon(":/icons/reset.svg");
    m_resetAction = new QAction(resetIcon, "Reset (F3)", this);
    connect(m_resetAction, &QAction::triggered, this, &ProcessorTab::reset);
    m_resetAction->setShortcut(QKeySequence("F3"));
    m_resetAction->setToolTip("Reset the simulator (F3)");
    controlToolbar->addAction(m_resetAction);

    const QIcon reverseIcon = QIcon(":/icons/reverse.svg");
    m_reverseAction = new QAction(reverseIcon, "Reverse (F4)", this);
    connect(m_reverseAction, &QAction::triggered, this, &ProcessorTab::reverse);
    m_reverseAction->setShortcut(QKeySequence("F4"));
    m_reverseAction->setToolTip("Undo a clock cycle (F4)");
    controlToolbar->addAction(m_reverseAction);

    const QIcon clockIcon = QIcon(":/icons/step.svg");
    m_clockAction = new QAction(clockIcon, "Clock (F5)", this);
    connect(m_clockAction, &QAction::triggered, this, &ProcessorTab::clock);
    m_clockAction->setShortcut(QKeySequence("F5"));
    m_clockAction->setToolTip("Clock the circuit (F5)");
    controlToolbar->addAction(m_clockAction);

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ProcessorTab::clock);

    const QIcon startAutoClockIcon = QIcon(":/icons/step-clock.svg");
    const QIcon stopAutoTimerIcon = QIcon(":/icons/stop-clock.svg");
    m_autoClockAction = new QAction(startAutoClockIcon, "Auto clock (F6)", this);
    m_autoClockAction->setShortcut(QKeySequence("F6"));
    m_autoClockAction->setToolTip("Clock the circuit with the selected frequency (F6)");
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
    controlToolbar->addAction(m_autoClockAction);

    m_autoClockInterval = new QSpinBox(this);
    m_autoClockInterval->setRange(1, 10000);
    m_autoClockInterval->setSuffix(" ms");
    m_autoClockInterval->setToolTip("Auto clock interval");
    connect(m_autoClockInterval, qOverload<int>(&QSpinBox::valueChanged),
            [timer](int msec) { timer->setInterval(msec); });
    m_autoClockInterval->setValue(100);
    controlToolbar->addWidget(m_autoClockInterval);

    const QIcon runIcon = QIcon(":/icons/run.svg");
    m_runAction = new QAction(runIcon, "Run (F8)", this);
    m_runAction->setShortcut(QKeySequence("F8"));
    m_runAction->setCheckable(true);
    m_runAction->setChecked(false);
    m_runAction->setToolTip(
        "Execute simulator without updating UI (fast execution) (F8).\n Running will stop once the program exits or a "
        "breakpoint is hit.");
    connect(m_runAction, &QAction::toggled, this, &ProcessorTab::run);
    controlToolbar->addAction(m_runAction);

    // Setup processor-tab only actions
    const QIcon tagIcon = QIcon(":/icons/tag.svg");
    m_displayValuesAction = new QAction(tagIcon, "Display signal values", this);
    m_displayValuesAction->setCheckable(true);
    m_displayValuesAction->setChecked(false);
    connect(m_displayValuesAction, &QAction::triggered, m_vsrtlWidget, &vsrtl::VSRTLWidget::setOutputPortValuesVisible);
    m_toolbar->addAction(m_displayValuesAction);

    const QIcon tableIcon = QIcon(":/icons/spreadsheet.svg");
    m_stageTableAction = new QAction(tableIcon, "Show stage table", this);
    connect(m_stageTableAction, &QAction::triggered, this, &ProcessorTab::showStageTable);
    m_toolbar->addAction(m_stageTableAction);
}

void ProcessorTab::updateStatistics() {
    static auto lastUpdateTime = std::chrono::system_clock::now();
    static long long lastCycleCount = ProcessorHandler::get()->getProcessor()->getCycleCount();

    const auto timeNow = std::chrono::system_clock::now();
    const auto cycleCount = ProcessorHandler::get()->getProcessor()->getCycleCount();
    const auto instrsRetired = ProcessorHandler::get()->getProcessor()->getInstructionsRetired();
    const auto timeDiff =
        std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - lastUpdateTime).count() / 1000.0;  // in seconds
    const auto cycleDiff = cycleCount - lastCycleCount;

    // Cycle count
    m_ui->cycleCount->setText(QString::number(cycleCount));
    // Instructions retired
    m_ui->instructionsRetired->setText(QString::number(instrsRetired));
    QString cpiText, ipcText;
    if (cycleCount != 0 && instrsRetired != 0) {
        const double cpi = static_cast<double>(cycleCount) / static_cast<double>(instrsRetired);
        const double ipc = 1 / cpi;
        cpiText = QString::number(cpi, 'g', 3);
        ipcText = QString::number(ipc, 'g', 3);
    }
    // CPI & IPC
    m_ui->cpi->setText(cpiText);
    m_ui->ipc->setText(ipcText);

    // Clock rate
    const double clockRate = static_cast<double>(cycleDiff) / timeDiff;
    m_ui->clockRate->setText(convertToSIUnits(clockRate) + "Hz");

    // Record timestamp values
    lastUpdateTime = timeNow;
    lastCycleCount = cycleCount;
}

void ProcessorTab::pause() {
    m_autoClockAction->setChecked(false);
    m_runAction->setChecked(false);
    m_reverseAction->setEnabled(m_vsrtlWidget->isReversible());
}

void ProcessorTab::fitToView() {
    m_vsrtlWidget->zoomToFit();
}

void ProcessorTab::loadProcessorToWidget(const Layout& layout) {
    ProcessorHandler::get()->loadProcessorToWidget(m_vsrtlWidget);

    // Construct stage instruction labels
    auto* topLevelComponent = m_vsrtlWidget->getTopLevelComponent();

    m_stageInstructionLabels.clear();
    const auto& proc = ProcessorHandler::get()->getProcessor();
    for (unsigned i = 0; i < proc->stageCount(); i++) {
        auto* stagelabel = new vsrtl::Label("-", topLevelComponent);
        stagelabel->setPointSize(14);
        m_stageInstructionLabels[i] = stagelabel;
    }
    loadLayout(layout);
    updateInstructionLabels();
    fitToView();
}

void ProcessorTab::processorSelection() {
    m_autoClockAction->setChecked(false);
    ProcessorSelectionDialog diag;
    if (diag.exec()) {
        // New processor model was selected
        m_vsrtlWidget->clearDesign();
        m_stageInstructionLabels.clear();
        ProcessorHandler::get()->selectProcessor(diag.getSelectedId(), diag.getRegisterInitialization());
        loadProcessorToWidget(diag.getSelectedLayout());
        m_vsrtlWidget->reset();
        updateInstructionModel();
        m_ui->registerWidget->updateModel();

        // Retrigger value display action if enabled
        if (m_displayValuesAction->isChecked()) {
            m_vsrtlWidget->setOutputPortValuesVisible(true);
        }
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
    m_reverseAction->setEnabled(m_vsrtlWidget->isReversible());
    m_resetAction->setEnabled(true);
    m_stageTableAction->setEnabled(!m_hasRun);
}

void ProcessorTab::updateInstructionLabels() {
    const auto& proc = ProcessorHandler::get()->getProcessor();
    for (unsigned i = 0; i < proc->stageCount(); i++) {
        if (!m_stageInstructionLabels.count(i))
            continue;
        const auto stageInfo = proc->stageInfo(i);
        auto* instrLabel = m_stageInstructionLabels.at(i);
        QString instrString;
        if (stageInfo.state != StageInfo::State::None) {
            instrString = stageInfo.state == StageInfo::State::Flushed ? "nop (flush)" : "nop (stall)";
            instrLabel->setDefaultTextColor(Qt::red);
        } else if (stageInfo.stage_valid) {
            instrString = ProcessorHandler::get()->parseInstrAt(stageInfo.pc);
            instrLabel->setDefaultTextColor(QColor());
        }
        instrLabel->setText(instrString);
    }
}

void ProcessorTab::reset() {
    m_hasRun = false;
    m_autoClockAction->setChecked(false);
    m_vsrtlWidget->reset();
    m_stageModel->reset();
    emit update();
    emit processorWasReset();

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
    m_statUpdateTimer->stop();
    emit update();
}

void ProcessorTab::run(bool state) {
    m_hasRun = true;
    // Stop any currently exeuting auto-clocking
    if (m_autoClockAction->isChecked()) {
        m_autoClockAction->setChecked(false);
    }
    if (state) {
        ProcessorHandler::get()->run();
        m_statUpdateTimer->start();
    } else {
        ProcessorHandler::get()->stop();
        m_statUpdateTimer->stop();
    }

    // Enable/Disable all actions based on whether the processor is running.
    m_selectProcessorAction->setEnabled(!state);
    m_clockAction->setEnabled(!state);
    m_autoClockAction->setEnabled(!state);
    m_reverseAction->setEnabled(!state);
    m_resetAction->setEnabled(!state);
    m_displayValuesAction->setEnabled(!state);
    m_stageTableAction->setEnabled(false);

    // Disable the entire processortab, disallowing interactions with widgets
    setEnabled(!state);
}

void ProcessorTab::reverse() {
    m_vsrtlWidget->reverse();
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
    m_reverseAction->setEnabled(m_vsrtlWidget->isReversible());

    emit update();
}

void ProcessorTab::showStageTable() {
    auto w = StageTableWidget(m_stageModel);
    w.exec();
}
}  // namespace Ripes
