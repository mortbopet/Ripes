#include "processortab.h"
#include <QScrollBar>
#include "instructionmodel.h"
#include "pipelinewidget.h"
#include "rundialog.h"
#include "ui_processortab.h"

#include <QFileDialog>

#include "parser.h"
#include "pipeline.h"

ProcessorTab::ProcessorTab(QWidget* parent) : QWidget(parent), m_ui(new Ui::ProcessorTab) {
    m_ui->setupUi(this);

    // Setup buttons
    connect(m_ui->start, &QPushButton::toggled, this, &ProcessorTab::toggleTimer);

    // Setup execution speed slider
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

    // Initially, no file is loaded, disable run, step and reset buttons
    m_ui->reset->setEnabled(false);
    m_ui->step->setEnabled(false);
    m_ui->run->setEnabled(false);
    m_ui->start->setEnabled(false);
    m_ui->table->setEnabled(false);
}

void ProcessorTab::toggleTimer(bool state) {
    const static QString pauseText = QLatin1String("Stop autostepping (F5)");
    const static QString startText = QLatin1String("Start autostepping (F5)");
    if (state) {
        m_ui->start->setText(pauseText);
        m_ui->start->setShortcut(Qt::Key_F5);  // The shortcut is for some reason cleared when editing the button text
        m_timer.start();
    } else {
        m_ui->start->setText(startText);
        m_ui->start->setShortcut(Qt::Key_F5);
        m_ui->start->setChecked(false);
        m_timer.stop();
    };
}

void ProcessorTab::restart() {
    // Invoked when changes to binary simulation file has been made
    emit update();
    bool pipelineReady = Pipeline::getPipeline()->getTextSize() > 0;

    m_ui->step->setEnabled(pipelineReady);
    m_ui->run->setEnabled(pipelineReady);
    m_ui->reset->setEnabled(pipelineReady);
    m_ui->start->setEnabled(pipelineReady);
    m_ui->table->setEnabled(pipelineReady);
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

void ProcessorTab::on_expandView_clicked() {
    m_ui->pipelineWidget->expandToView();
}

void ProcessorTab::on_displayValues_toggled(bool checked) {
    m_ui->pipelineWidget->displayAllValues(checked);
}

void ProcessorTab::on_run_clicked() {
    auto pipeline = Pipeline::getPipeline();
    RunDialog dialog(this);
    if (pipeline->isReady()) {
        if (dialog.exec() && pipeline->isFinished()) {
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
    m_ui->table->setEnabled(true);
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

void ProcessorTab::on_step_clicked() {
    auto pipeline = Pipeline::getPipeline();
    auto state = pipeline->step();

    const auto ecallVal = pipeline->checkEcall(true);
    if (ecallVal.first != Pipeline::ECALL::none) {
        handleEcall(ecallVal);
    }

    emit update();

    // Pipeline has finished executing
    if (pipeline->isFinished() || (state == 1 && ecallVal.first == Pipeline::ECALL::exit)) {
        m_ui->step->setEnabled(false);
        m_ui->start->setEnabled(false);
        m_ui->run->setEnabled(false);
        toggleTimer(false);
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

void ProcessorTab::on_zoomIn_clicked() {
    m_ui->pipelineWidget->zoomIn();
}

void ProcessorTab::on_zoomOut_clicked() {
    m_ui->pipelineWidget->zoomOut();
}

void ProcessorTab::on_save_clicked() {
    QFileDialog dialog;
    dialog.setNameFilter("*.png");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    if (dialog.exec()) {
        auto files = dialog.selectedFiles();
        if (files.length() == 1) {
            auto scene = m_ui->pipelineWidget->scene();
            QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
            image.fill(Qt::white);
            QPainter painter(&image);
            painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                                   QPainter::SmoothPixmapTransform);
            scene->render(&painter, QRectF(), QRect(), Qt::IgnoreAspectRatio);
            image.save(files[0]);
        }
    }
}

void ProcessorTab::on_table_clicked() {
    // Setup pipeline table window
    PipelineTable window;
    PipelineTableModel model;
    window.setModel(&model);
    window.exec();
}
