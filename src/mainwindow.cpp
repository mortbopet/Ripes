#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutwidget.h"
#include "defines.h"
#include "edittab.h"
#include "memorytab.h"
#include "parser.h"
#include "processorhandler.h"
#include "processortab.h"
#include "registerwidget.h"

#include "fancytabbar/fancytabbar.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextStream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);
    setWindowTitle("Ripes");
    setWindowIcon(QIcon(":/icons/logo.svg"));
    showMaximized();

    // Setup processor handler
    m_processorHandler = new ProcessorHandler();

    // Create tabs
    m_stackedTabs = new QStackedWidget(this);
    m_ui->centrallayout->addWidget(m_stackedTabs);

    auto* tb = addToolBar("Edit");
    tb->setVisible(false);
    m_editTab = new EditTab(tb, this);
    m_stackedTabs->insertWidget(0, m_editTab);

    tb = addToolBar("Processor");
    tb->setVisible(false);
    m_processorTab = new ProcessorTab(*m_processorHandler, tb, this);
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

    connect(m_stackedTabs, &QStackedWidget::currentChanged, this, &MainWindow::tabChanged);
    tabChanged();

    // Setup tab pointers
    m_processorTab->initRegWidget();
    m_processorTab->initInstructionView();
    m_memoryTab->initMemoryTab();

    setupMenus();

    // setup and connect widgets
    connect(m_processorTab, &ProcessorTab::update, this, &MainWindow::updateMemoryTab);
    connect(m_editTab, &EditTab::updateSimulator, [this] { emit update(); });
    connect(this, &MainWindow::update, m_processorTab, &ProcessorTab::restart);
    connect(this, &MainWindow::updateMemoryTab, m_memoryTab, &MemoryTab::update);
    connect(m_stackedTabs, &QStackedWidget::currentChanged, m_memoryTab, &MemoryTab::update);
    connect(m_editTab, &EditTab::programChanged, m_processorHandler, &ProcessorHandler::loadProgram);
    connect(m_processorHandler, &ProcessorHandler::reqProcessorReset, m_processorTab, &ProcessorTab::reset);
    connect(m_processorHandler, &ProcessorHandler::reqReloadProgram, m_editTab, &EditTab::emitProgramChanged);

    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionOpen_wiki, &QAction::triggered, this, &MainWindow::wiki);
}

void MainWindow::tabChanged() {
    // Enable the toolbar associated with the currently selected tab
    auto* tab = dynamic_cast<RipesTab*>(m_stackedTabs->currentWidget());
    for (int i = 0; i < m_stackedTabs->count(); i++) {
        auto* w = dynamic_cast<RipesTab*>(m_stackedTabs->widget(i));
        auto* tb = w->getToolbar();
        tb->setVisible(w == tab && (tb->children().length() != 0));
    }
}

void MainWindow::setupMenus() {
    // Edit actions
    const QIcon newIcon = QIcon(":/icons/file.svg");
    auto* newAction = new QAction(newIcon, "New Assembly Program", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newProgramTriggered);
    m_ui->menuFile->addAction(newAction);
    m_editTab->getToolbar()->addAction(newAction);

    const QIcon loadIcon = QIcon(":/icons/loadfile.svg");
    auto* loadAction = new QAction(loadIcon, "Load file", this);
    loadAction->setShortcut(QKeySequence::Open);
    connect(loadAction, &QAction::triggered, [=] { this->loadAssemblyFile(); });
    m_editTab->getToolbar()->addAction(loadAction);
    m_ui->menuFile->addAction(loadAction);

    m_ui->menuFile->addSeparator();

    auto* examplesMenu = m_ui->menuFile->addMenu("Load Example...");
    setupExamplesMenu(examplesMenu);

    m_ui->menuFile->addSeparator();

    const QIcon saveIcon = QIcon(":/icons/save.svg");
    auto* saveAction = new QAction(saveIcon, "Save File", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFilesTriggered);
    m_editTab->getToolbar()->addAction(saveAction);
    m_ui->menuFile->addAction(saveAction);

    const QIcon saveAsIcon = QIcon(":/icons/saveas.svg");
    auto* saveAsAction = new QAction(saveAsIcon, "Save File As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFilesAsTriggered);
    m_editTab->getToolbar()->addAction(saveAsAction);
    m_ui->menuFile->addAction(saveAsAction);

    m_ui->menuFile->addSeparator();

    const QIcon exitIcon = QIcon(":/icons/cancel.svg");
    auto* exitAction = new QAction(exitIcon, "Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    m_ui->menuFile->addAction(exitAction);
}

void MainWindow::run() {
    // Function for triggering the run dialog from unit tests
    m_processorTab->run();
}

MainWindow::~MainWindow() {
    delete m_ui;
}

#include <QDebug>
void MainWindow::setupExamplesMenu(QMenu* parent) {
    auto binaryExamples = QDir(":/examples/binary/").entryList(QDir::Files);
    auto assemblyExamples = QDir(":/examples/assembly/").entryList(QDir::Files);

    // Load examples
    if (!binaryExamples.isEmpty()) {
        auto* binaryExampleMenu = new QMenu();
        binaryExampleMenu->setTitle("Binary");
        for (const auto& fileName : binaryExamples) {
            binaryExampleMenu->addAction(fileName, [=] {
                this->loadBinaryFile(QString(":/examples/binary/") + fileName);
                m_currentFile = QString();
            });
        }
        // Add binary example menu to example menu
        parent->addMenu(binaryExampleMenu);
    }

    if (!assemblyExamples.isEmpty()) {
        auto* assemblyExampleMenu = new QMenu();
        assemblyExampleMenu->setTitle("Assembly");
        for (const auto& fileName : assemblyExamples) {
            assemblyExampleMenu->addAction(fileName, [=] {
                this->loadAssemblyFile(QString(":/examples/assembly/") + fileName);
                m_currentFile = QString();
            });
        }
        // Add binary example menu to example menu
        parent->addMenu(assemblyExampleMenu);
    }
}

void MainWindow::exit() {
    close();
}

void MainWindow::loadBinaryFile(QString filename) {
    m_editTab->setInputMode(false);
    m_processorTab->restart();
    Parser::getParser()->loadBinaryFile(filename);
    m_editTab->setDisassemblerText();
    emit update();
}

void MainWindow::loadAssemblyFile(QString filename) {
    if (filename == QString()) {
        filename = QFileDialog::getOpenFileName(this, "Open assembly file", "", "Assembly file (*.s *.as *.asm)");
    }
    if (!filename.isNull()) {
        // ... load file
        QFile file(filename);
        m_editTab->setInputMode(true);
        Parser::getParser()->clear();
        m_editTab->clear();
        m_processorTab->restart();
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_editTab->setAssemblyText(file.readAll());
            file.close();
        }
        m_currentFile = filename;
        m_editTab->setDisassemblerText();
    }
}

void MainWindow::about() {
    AboutWidget about;
    about.exec();
}

void MainWindow::wiki() {
    QDesktopServices::openUrl(QUrl(QString("https://github.com/mortbopet/Ripes/wiki")));
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
    if (m_currentFile.isEmpty()) {
        saveFilesAsTriggered();
    }

    // if (m_ui->actionSave_Source->isChecked()) {
    {
        QFile file(m_currentFile);
        writeTextFile(file, m_editTab->getAssemblyText());
    }
    //}

    // if (m_ui->actionSave_Disassembled->isChecked()) {
    {
        QFile file(removeFileExt(m_currentFile) + "_dis.s");
        writeTextFile(file, Parser::getParser()->getDisassembledRepr());
    }
    //}

    // QAction* binaryStoreAction = m_binaryStoreAction->checkedAction();
    // if (binaryStoreAction == m_ui->actionSave_as_flat_binary) {
    {
        QFile file(removeFileExt(m_currentFile) + ".bin");
        writeBinaryFile(file, m_editTab->getBinaryData());
    }
    //} else if (binaryStoreAction == m_ui->actionSave_as_text) {

    {
        QFile file(removeFileExt(m_currentFile) + "_bin.txt");
        writeTextFile(file, Parser::getParser()->getBinaryRepr());
    }
    //}
}

void MainWindow::saveFilesAsTriggered() {
    QFileDialog dialog(this);
    dialog.setNameFilter("*.as *.s");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix(".s");
    dialog.setModal(true);
    if (dialog.exec()) {
        m_currentFile = dialog.selectedFiles()[0];
        saveFilesTriggered();
    }
}

void MainWindow::newProgramTriggered() {
    QMessageBox mbox;
    mbox.setWindowTitle("New Program...");
    mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (!m_editTab->getAssemblyText().isEmpty() && m_currentFile.isEmpty()) {
        // User wrote a program but did not save it to a file yet
        mbox.setText("Save program before creating new file?");
        auto ret = mbox.exec();
        switch (ret) {
            case QMessageBox::Yes: {
                saveFilesAsTriggered();
                break;
            }
            case QMessageBox::No: {
                break;
            }
            case QMessageBox::Cancel: {
                return;
            }
        }
    } else if (!m_currentFile.isEmpty()) {
        // User previously stored a program but may have updated in the meantime - prompt to ask whether the program
        // should be stored to the current file name
        mbox.setText(QString("Save program \"%1\" before creating new file?").arg(m_currentFile));
        auto ret = mbox.exec();
        switch (ret) {
            case QMessageBox::Yes: {
                saveFilesTriggered();
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
    m_currentFile.clear();
    m_editTab->newProgram();
}
