#include "mainwindow.h"
#include "QFileDialog"
#include "ui_mainwindow.h"

#include "aboutwidget.h"

#include <QDesktopServices>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include "parser.h"

#include "programfiletab.h"
#include "registerwidget.h"

#include "defines.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);
    setWindowTitle("Ripes");
    setWindowIcon(QIcon(":/icons/logo.svg"));
    showMaximized();

    // Setup tab bar
    m_ui->tabbar->addFancyTab(QIcon(":/icons/binary-code.svg"), "Editor");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/cpu.svg"), "Processor");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/ram-memory.svg"), "Memory");
    // m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/icons/server.svg")), "Cache");
    // m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/icons/graph.svg")), "Results");
    connect(m_ui->tabbar, &FancyTabBar::activeIndexChanged, m_ui->stackedWidget, &QStackedWidget::setCurrentIndex);
    m_ui->tabbar->setActiveIndex(0);

    // Setup tab pointers
    m_ui->processortab->initRegWidget();
    m_ui->processortab->initInstructionView();
    m_ui->memorytab->initMemoryTab();

    // setup example projects
    setupExamples();

    // setup and connect widgets
    connect(m_ui->programfiletab, &ProgramfileTab::loadBinaryFile, this,
            &MainWindow::on_actionLoadBinaryFile_triggered);
    connect(m_ui->programfiletab, &ProgramfileTab::loadAssemblyFile, this,
            &MainWindow::on_actionLoadAssemblyFile_triggered);

    connect(m_ui->processortab, &ProcessorTab::update, this, &MainWindow::updateMemoryTab);
    connect(m_ui->programfiletab, &ProgramfileTab::updateSimulator, [this] { emit update(); });
    connect(this, &MainWindow::update, m_ui->processortab, &ProcessorTab::restart);
    connect(this, &MainWindow::updateMemoryTab, m_ui->memorytab, &MemoryTab::update);
    connect(m_ui->stackedWidget, &QStackedWidget::currentChanged, m_ui->memorytab, &MemoryTab::update);
}

MainWindow::~MainWindow() {
    delete m_ui;
}

#include <QDebug>
void MainWindow::setupExamples() {
    auto binaryExamples = QDir(":/examples/binary/").entryList(QDir::Files);
    auto assemblyExamples = QDir(":/examples/assembly/").entryList(QDir::Files);

    // Load examples
    if (!binaryExamples.isEmpty()) {
        auto* binaryExampleMenu = new QMenu();
        binaryExampleMenu->setTitle("Binary");
        for (const auto& fileName : binaryExamples) {
            binaryExampleMenu->addAction(fileName,
                                         [=] { this->loadBinaryFile(QString(":/examples/binary/") + fileName); });
        }
        // Add binary example menu to example menu
        m_ui->menuExamples->addMenu(binaryExampleMenu);
    }

    if (!assemblyExamples.isEmpty()) {
        auto* assemblyExampleMenu = new QMenu();
        assemblyExampleMenu->setTitle("Assembly");
        for (const auto& fileName : assemblyExamples) {
            assemblyExampleMenu->addAction(fileName,
                                           [=] { this->loadAssemblyFile(QString(":/examples/assembly/") + fileName); });
        }
        // Add binary example menu to example menu
        m_ui->menuExamples->addMenu(assemblyExampleMenu);
    }
}

void MainWindow::on_actionexit_triggered() {
    close();
}

void MainWindow::on_actionLoadBinaryFile_triggered() {
    auto filename = QFileDialog::getOpenFileName(this, "Open binary file", "", "Binary file (*)");
    if (!filename.isNull())
        loadBinaryFile(filename);
}

void MainWindow::on_actionLoadAssemblyFile_triggered() {
    auto filename = QFileDialog::getOpenFileName(this, "Open assembly file", "", "Assembly file (*.s *.as *.asm)");
    if (!filename.isNull())
        loadAssemblyFile(filename);
}

void MainWindow::loadBinaryFile(QString filename) {
    m_ui->programfiletab->setTimerEnabled(false);
    m_ui->programfiletab->setInputMode(false);
    m_ui->processortab->restart();
    Parser::getParser()->loadBinaryFile(filename);
    m_ui->programfiletab->setDisassemblerText();
    emit update();
}

void MainWindow::loadAssemblyFile(QString fileName) {
    // ... load file
    QFile file(fileName);
    m_ui->programfiletab->setInputMode(true);
    m_ui->programfiletab->setTimerEnabled(true);
    Parser::getParser()->clear();
    m_ui->programfiletab->clearOutputArray();
    m_ui->processortab->restart();
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_ui->programfiletab->setAssemblyText(file.readAll());
        file.close();
    }
    m_ui->programfiletab->setDisassemblerText();
}

void MainWindow::on_actionAbout_triggered() {
    AboutWidget about;
    about.exec();
}

void MainWindow::on_actionOpen_documentation_triggered() {
    // Look for documentation pdf
    QDesktopServices::openUrl(QUrl(QString("https://github.com/mortbopet/Ripes")));
}

void MainWindow::on_actionSave_Assembly_File_triggered() {
    QFileDialog dialog;
    dialog.setNameFilter("*.as *.s");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec()) {
        auto files = dialog.selectedFiles();
        if (files.length() == 1) {
            QFile file(files[0]);
            if (file.open(QIODevice::ReadWrite)) {
                QTextStream stream(&file);
                stream << m_ui->programfiletab->getAssemblyText();
                file.close();
            }
        }
    }
}
