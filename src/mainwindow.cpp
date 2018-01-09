#include "mainwindow.h"
#include "QFileDialog"
#include "ui_mainwindow.h"

#include "aboutwidget.h"

#include <QDesktopServices>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include "parser.h"

#include "programfiletab.h"
#include "registerwidget.h"

#include "defines.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);
    setWindowTitle("RISC-V-SIM");
    setWindowIcon(QIcon(QPixmap(":/icons/logo.png")));
    showMaximized();

    // Setup tab bar
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/icons/binary-code.svg")), "Code");
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/icons/cpu.svg")), "Processor");
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/icons/ram-memory.svg")), "Memory");
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/icons/server.svg")), "Cache");
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/icons/graph.svg")), "Results");
    connect(m_ui->tabbar, &FancyTabBar::activeIndexChanged, m_ui->stackedWidget, &QStackedWidget::setCurrentIndex);
    m_ui->tabbar->setActiveIndex(1);

    // Setup splitter such that consoles are always as small as possible
    auto splitterSize = m_ui->splitter->size();
    m_ui->splitter->setSizes(QList<int>() << splitterSize.height() - (m_ui->consoles->minimumHeight() - 1)
                                          << (m_ui->consoles->minimumHeight() + 1));

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
    connect(m_ui->actionShow_consoles, &QAction::triggered, [=](bool state) {
        if (state) {
            m_ui->consoles->show();
        } else {
            m_ui->consoles->hide();
        };
    });
    connect(this, &MainWindow::update, m_ui->processortab, &ProcessorTab::restart);
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::setupExamples() {
    // All .bin and .asm files in folder examples/.. will be added to the list of binary and assembly examples that can
    // be selected through the menu
    auto binaryExamples = QDir("examples/binary/").entryList(QDir::Files);
    auto assemblyExamples = QDir("examples/assembly/").entryList(QDir::Files);

    // Load examples
    if (!binaryExamples.isEmpty()) {
        QMenu* binaryExampleMenu = new QMenu();
        binaryExampleMenu->setTitle("Binary");
        for (const auto& fileName : binaryExamples) {
            binaryExampleMenu->addAction(
                fileName, [=] { this->loadBinaryFile(QDir::currentPath() + QString("/examples/binary/") + fileName); });
        }
        // Add binary example menu to example menu
        m_ui->menuExamples->addMenu(binaryExampleMenu);
    }

    if (!assemblyExamples.isEmpty()) {
        QMenu* assemblyExampleMenu = new QMenu();
        assemblyExampleMenu->setTitle("Assembly");
        for (const auto& fileName : assemblyExamples) {
            assemblyExampleMenu->addAction(fileName, [=] {
                this->loadAssemblyFile(QDir::currentPath() + QString("/examples/assembly/") + fileName);
            });
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
}

void MainWindow::on_actionLoadAssemblyFile_triggered() {
    auto filename = QFileDialog::getOpenFileName(this, "Open assembly file", "", "Assembly file (*.s *.as *.asm)");
}

void MainWindow::loadBinaryFile(QString filename) {
    const QString& output = Parser::getParser()->loadBinaryFile(filename);
    m_ui->programfiletab->setBinaryText(output);
    emit update();
}

void MainWindow::loadAssemblyFile(QString fileName) {
    // ... load file
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_ui->programfiletab->setAssemblyText(file.readAll());
        file.close();
    }
}

void MainWindow::on_actionAbout_triggered() {
    AboutWidget about;
    about.exec();
}

void MainWindow::on_actionOpen_documentation_triggered() {
    // Look for documentation pdf
    QFileInfo file("riscvsim_doc.pdf");
    if (file.exists()) {
        QDesktopServices::openUrl(QUrl("riscvsim_doc.pdf"));
    } else {
        QMessageBox info;
        info.setIcon(QMessageBox::Warning);
        info.setText("Could not locate documentation file");
        info.exec();
    }
}
