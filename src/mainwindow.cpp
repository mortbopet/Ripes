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

    m_binaryStoreAction = new QActionGroup(this);
    m_binaryStoreAction->addAction(m_ui->actionSave_as_flat_binary);
    m_binaryStoreAction->addAction(m_ui->actionSave_as_text);
    m_binaryStoreAction->addAction(m_ui->actionDisable);
    m_binaryStoreAction->setExclusive(true);
    m_ui->actionDisable->setChecked(true);

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

void MainWindow::run() {
    // Function for triggering the run dialog from unit tests
    m_ui->processortab->on_run_clicked();
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
            binaryExampleMenu->addAction(fileName, [=] {
                this->loadBinaryFile(QString(":/examples/binary/") + fileName);
                m_currentFile = QString();
            });
        }
        // Add binary example menu to example menu
        m_ui->menuExamples->addMenu(binaryExampleMenu);
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
    m_currentFile = fileName;
    m_ui->programfiletab->setDisassemblerText();
}

void MainWindow::on_actionAbout_triggered() {
    AboutWidget about;
    about.exec();
}

void MainWindow::on_actionOpen_wiki_triggered() {
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

void MainWindow::on_actionSave_Files_triggered() {
    if (m_currentFile.isEmpty()) {
        on_actionSave_Files_As_triggered();
    }

    if (m_ui->actionSave_Source->isChecked()) {
        QFile file(m_currentFile);
        writeTextFile(file, m_ui->programfiletab->getAssemblyText());
    }

    if (m_ui->actionSave_Disassembled->isChecked()) {
        QFile file(removeFileExt(m_currentFile) + "_dis.s");
        writeTextFile(file, Parser::getParser()->getDisassembledRepr());
    }

    QAction* binaryStoreAction = m_binaryStoreAction->checkedAction();
    if (binaryStoreAction == m_ui->actionSave_as_flat_binary) {
        QFile file(removeFileExt(m_currentFile) + ".bin");
        writeBinaryFile(file, m_ui->programfiletab->getBinaryData());
    } else if (binaryStoreAction == m_ui->actionSave_as_text) {
        QFile file(removeFileExt(m_currentFile) + "_bin.txt");
        writeTextFile(file, Parser::getParser()->getBinaryRepr());
    }
}

void MainWindow::on_actionSave_Files_As_triggered() {
    QFileDialog dialog;
    dialog.setNameFilter("*.as *.s");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix(".s");
    if (dialog.exec()) {
        m_currentFile = dialog.selectedFiles()[0];
        on_actionSave_Files_triggered();
    }
}

void MainWindow::on_actionNew_Program_triggered() {
    QMessageBox mbox;
    mbox.setWindowTitle("New Program...");
    mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (!m_ui->programfiletab->getAssemblyText().isEmpty() && m_currentFile.isEmpty()) {
        // User wrote a program but did not save it to a file yet
        mbox.setText("Save program before creating new file?");
        auto ret = mbox.exec();
        switch (ret) {
            case QMessageBox::Yes: {
                on_actionSave_Files_As_triggered();
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
                on_actionSave_Files_triggered();
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
    m_ui->programfiletab->newProgram();
}
