#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "defines.h"
#include "edittab.h"
#include "loaddialog.h"
#include "memorytab.h"
#include "parser.h"
#include "processorhandler.h"
#include "processortab.h"
#include "registerwidget.h"
#include "savedialog.h"
#include "version/version.h"

#include "fancytabbar/fancytabbar.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextStream>

namespace Ripes {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);
    setWindowTitle("Ripes");
    setWindowIcon(QIcon(":/icons/logo.svg"));
    m_ui->actionOpen_wiki->setIcon(QIcon(":/icons/info.svg"));
    showMaximized();

    // Initialize processor handler
    ProcessorHandler::get();

    // Create tabs
    m_stackedTabs = new QStackedWidget(this);
    m_ui->centrallayout->addWidget(m_stackedTabs);

    auto* tb = addToolBar("Edit");
    tb->setVisible(false);
    m_editTab = new EditTab(tb, this);
    m_stackedTabs->insertWidget(0, m_editTab);

    tb = addToolBar("Processor");
    tb->setVisible(true);
    m_processorTab = new ProcessorTab(tb, this);
    m_stackedTabs->insertWidget(1, m_processorTab);

    tb = addToolBar("Processor");
    tb->setVisible(false);
    m_memoryTab = new MemoryTab(tb, this);
    m_stackedTabs->insertWidget(2, m_memoryTab);

    // Setup tab bar
    m_ui->tabbar->addFancyTab(QIcon(":/icons/binary-code.svg"), "Editor");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/cpu.svg"), "Processor");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/ram-memory.svg"), "Memory");
    connect(m_ui->tabbar, &FancyTabBar::activeIndexChanged, m_stackedTabs, &QStackedWidget::setCurrentIndex);
    m_ui->tabbar->setActiveIndex(0);

    setupMenus();

    // setup and connect widgets
    connect(m_processorTab, &ProcessorTab::update, this, &MainWindow::updateMemoryTab);
    connect(this, &MainWindow::update, m_processorTab, &ProcessorTab::restart);
    connect(this, &MainWindow::updateMemoryTab, m_memoryTab, &MemoryTab::update);
    connect(m_stackedTabs, &QStackedWidget::currentChanged, m_memoryTab, &MemoryTab::update);
    connect(m_editTab, &EditTab::programChanged, ProcessorHandler::get(), &ProcessorHandler::loadProgram);
    connect(m_editTab, &EditTab::editorStateChanged, [=] { this->m_hasSavedFile = false; });

    connect(ProcessorHandler::get(), &ProcessorHandler::reqProcessorReset, m_processorTab, &ProcessorTab::reset);
    connect(ProcessorHandler::get(), &ProcessorHandler::reqReloadProgram, m_editTab, &EditTab::emitProgramChanged);
    connect(ProcessorHandler::get(), &ProcessorHandler::print, m_processorTab, &ProcessorTab::printToLog);
    connect(ProcessorHandler::get(), &ProcessorHandler::exit, m_processorTab, &ProcessorTab::processorFinished);
    connect(ProcessorHandler::get(), &ProcessorHandler::runFinished, m_processorTab, &ProcessorTab::runFinished);

    connect(m_ui->actionOpen_wiki, &QAction::triggered, this, &MainWindow::wiki);
    connect(m_ui->actionVersion, &QAction::triggered, this, &MainWindow::version);
}

void MainWindow::setupMenus() {
    // Edit actions
    const QIcon newIcon = QIcon(":/icons/file.svg");
    auto* newAction = new QAction(newIcon, "New Program", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newProgramTriggered);
    m_ui->menuFile->addAction(newAction);

    const QIcon loadIcon = QIcon(":/icons/loadfile.svg");
    auto* loadAction = new QAction(loadIcon, "Load Program", this);
    loadAction->setShortcut(QKeySequence::Open);
    connect(loadAction, &QAction::triggered, [=] { this->loadFileTriggered(); });
    m_ui->menuFile->addAction(loadAction);

    m_ui->menuFile->addSeparator();

    auto* examplesMenu = m_ui->menuFile->addMenu("Load Example...");
    setupExamplesMenu(examplesMenu);

    m_ui->menuFile->addSeparator();

    const QIcon saveIcon = QIcon(":/icons/save.svg");
    auto* saveAction = new QAction(saveIcon, "Save File", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFilesTriggered);
    connect(m_editTab, &EditTab::editorStateChanged, [saveAction](bool enabled) { saveAction->setEnabled(enabled); });
    m_ui->menuFile->addAction(saveAction);

    const QIcon saveAsIcon = QIcon(":/icons/saveas.svg");
    auto* saveAsAction = new QAction(saveAsIcon, "Save File As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFilesAsTriggered);
    connect(m_editTab, &EditTab::editorStateChanged,
            [saveAsAction](bool enabled) { saveAsAction->setEnabled(enabled); });
    m_ui->menuFile->addAction(saveAsAction);

    m_ui->menuFile->addSeparator();

    const QIcon exitIcon = QIcon(":/icons/cancel.svg");
    auto* exitAction = new QAction(exitIcon, "Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    m_ui->menuFile->addAction(exitAction);
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::setupExamplesMenu(QMenu* parent) {
    const auto assemblyExamples = QDir(":/examples/assembly/").entryList(QDir::Files);

    if (!assemblyExamples.isEmpty()) {
        for (const auto& fileName : assemblyExamples) {
            parent->addAction(fileName, [=] {
                LoadFileParams parms;
                parms.filepath = QString(":/examples/assembly/") + fileName;
                parms.type = FileType::Assembly;
                m_editTab->loadFile(parms);
                m_hasSavedFile = false;
            });
        }
    }
}

void MainWindow::exit() {
    close();
}

void MainWindow::loadFileTriggered() {
    LoadDialog diag;
    if (!diag.exec())
        return;

    m_editTab->loadFile(diag.getParams());
    m_hasSavedFile = false;
}

void MainWindow::wiki() {
    QDesktopServices::openUrl(QUrl(QString("https://github.com/mortbopet/Ripes/wiki")));
}

void MainWindow::version() {
    QMessageBox aboutDialog(this);
    aboutDialog.setText("Ripes version: " + getRipesVersion());
    aboutDialog.exec();
}

namespace {
inline QString removeFileExt(const QString& file) {
    int lastPoint = file.lastIndexOf(".");
    return file.left(lastPoint);
}
void writeTextFile(QFile& file, const QString& data) {
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << data;
        file.close();
    }
}

void writeBinaryFile(QFile& file, const QByteArray& data) {
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
    }
}

}  // namespace

void MainWindow::saveFilesTriggered() {
    SaveDialog diag;
    if (!m_hasSavedFile) {
        diag.exec();
        m_hasSavedFile = true;
    }

    if (!diag.assemblyPath().isEmpty()) {
        QFile file(diag.assemblyPath());
        writeTextFile(file, m_editTab->getAssemblyText());
    }

    if (!diag.binaryPath().isEmpty()) {
        QFile file(diag.binaryPath());
        writeBinaryFile(file, m_editTab->getBinaryData());
    }
}

void MainWindow::saveFilesAsTriggered() {
    SaveDialog diag;
    auto ret = diag.exec();
    if (ret == QDialog::Rejected)
        return;
    m_hasSavedFile = true;
    saveFilesTriggered();
}

void MainWindow::newProgramTriggered() {
    QMessageBox mbox;
    mbox.setWindowTitle("New Program...");
    mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (!m_editTab->getAssemblyText().isEmpty() || m_hasSavedFile) {
        // User wrote a program but did not save it to a file yet
        mbox.setText("Save program before creating new file?");
        auto ret = mbox.exec();
        switch (ret) {
            case QMessageBox::Yes: {
                saveFilesTriggered();
                if (!m_hasSavedFile) {
                    // User must have rejected the save file dialog
                    return;
                }
                break;
            }
            case QMessageBox::No: {
                break;
            }
            case QMessageBox::Cancel: {
                return;
            }
        }
    }
    m_hasSavedFile = false;
    m_editTab->newProgram();
}

}  // namespace Ripes
