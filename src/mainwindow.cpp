#include "mainwindow.h"
#include "QFileDialog"
#include "ui_mainwindow.h"

#include <QIcon>
#include <QPushButton>

#include "programfiletab.h"
#include "registerwidget.h"
#include "runner.h"

#include "defines.h"

MainWindow::MainWindow(Runner* runnerPtr, Parser* parserPtr, QWidget* parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);
    setWindowTitle("RISC-V Simulator");
    setWindowIcon(QIcon(QPixmap(":/logos/logo.png")));

    m_runnerPtr = runnerPtr;
    m_parserPtr = parserPtr;

    // Setup tab bar
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/logos/binary-code.svg")),
                              "Code");
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/logos/cpu.svg")), "Processor");
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/logos/ram-memory.svg")),
                              "Memory");
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/logos/server.svg")), "Cache");
    m_ui->tabbar->addFancyTab(QIcon(QPixmap(":/logos/graph.svg")), "Results");
    connect(m_ui->tabbar, &FancyTabBar::activeIndexChanged, m_ui->stackedWidget,
            &QStackedWidget::setCurrentIndex);
    m_ui->tabbar->setActiveIndex(1);

    // Setup splitter such that consoles are always as small as possible
    auto splitterSize = m_ui->splitter->size();
    m_ui->splitter->setSizes(QList<int>()
                             << splitterSize.height() -
                                  (m_ui->consoles->minimumHeight() - 1)
                             << (m_ui->consoles->minimumHeight() + 1));

    // Setup tab pointers
    m_ui->cachetab->setRunnerCachePtr(m_runnerPtr->getRunnerCachePtr());
    m_ui->memorytab->setMemoryPtr(m_runnerPtr->getMemoryPtr());
    m_ui->memorytab->setRegPtr(m_runnerPtr->getRegPtr());
    m_ui->processortab->initRegWidget(m_runnerPtr->getRegPtr());

    m_ui->memorytab->init();

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
}

MainWindow::~MainWindow() { delete m_ui; }

void MainWindow::setupExamples() {
    auto binaryExamples = QDir("examples/binary/").entryList(QDir::Files);
    auto assemblyExamples = QDir("examples/assembly/").entryList(QDir::Files);

    // Load examples
    if (!binaryExamples.isEmpty()) {
        QMenu* binaryExampleMenu = new QMenu();
        binaryExampleMenu->setTitle("Binary");
        for (const auto& fileName : binaryExamples) {
            binaryExampleMenu->addAction(fileName, [=] {
                this->loadBinaryFile(QDir::currentPath() +
                                     QString("/examples/binary/") + fileName);
            });
        }
        // Add binary example menu to example menu
        m_ui->menuExamples->addMenu(binaryExampleMenu);
    }

    if (!assemblyExamples.isEmpty()) {
        QMenu* assemblyExampleMenu = new QMenu();
        assemblyExampleMenu->setTitle("Assembly");
        for (const auto& fileName : assemblyExamples) {
            assemblyExampleMenu->addAction(fileName, [=] {
                this->loadAssemblyFile(QDir::currentPath() +
                                       QString("/examples/assembly/") +
                                       fileName);
            });
        }
        // Add binary example menu to example menu
        m_ui->menuExamples->addMenu(assemblyExampleMenu);
    }
}

void MainWindow::on_actionexit_triggered() { close(); }

void MainWindow::on_actionLoadBinaryFile_triggered() {
    auto filename = QFileDialog::getOpenFileName(this, "Open binary file", "",
                                                 "Binary file (*)");
}

void MainWindow::on_actionLoadAssemblyFile_triggered() {
    auto filename = QFileDialog::getOpenFileName(
      this, "Open assembly file", "", "Assembly file (*.s *.as *.asm)");
}

void MainWindow::loadBinaryFile(QString fileName) {
    // ... load file
    QString output = "";
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray read = file.readAll();
        auto length = read.length();
        QDataStream in(&read, QIODevice::ReadOnly);
        char buffer[4];
        for (int i = 0; i < length; i += 4) {
            in.readRawData(buffer, 4);
            for (int j = 0; j < 4; j++) {
                output.append(QString()
                                .setNum((uint8_t)buffer[j], 2)
                                .rightJustified(8, '0'));
            }
            output.append("\n");
        }

        m_ui->programfiletab->setBinaryText(output);
        file.close();
    }
}
void MainWindow::loadAssemblyFile(QString fileName) {
    // ... load file
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_ui->programfiletab->setAssemblyText(file.readAll());
        file.close();
    }
}
