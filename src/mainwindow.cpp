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
#include "ripessettings.h"
#include "savedialog.h"
#include "settingsdialog.h"
#include "syscall/syscallviewer.h"
#include "syscall/systemio.h"
#include "version/version.h"

#include "fancytabbar/fancytabbar.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFontDatabase>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTemporaryFile>
#include <QTextStream>

namespace Ripes {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
    m_ui->setupUi(this);
    setWindowTitle("Ripes");
    setWindowIcon(QIcon(":/icons/logo.svg"));
    m_ui->actionOpen_wiki->setIcon(QIcon(":/icons/info.svg"));

    // Initialize processor handler
    ProcessorHandler::get();

    // Initialize fonts
    QFontDatabase::addApplicationFont(":/fonts/Inconsolata/Inconsolata-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inconsolata/Inconsolata-Bold.ttf");

    // Create tabs
    m_stackedTabs = new QStackedWidget(this);
    m_ui->centrallayout->addWidget(m_stackedTabs);

    m_controlToolbar = addToolBar("Simulator control");
    m_controlToolbar->setVisible(true);  // Always visible

    m_editToolbar = addToolBar("Edit");
    m_editToolbar->setVisible(false);
    m_editTab = new EditTab(m_editToolbar, this);
    m_stackedTabs->insertWidget(0, m_editTab);

    m_processorToolbar = addToolBar("Processor");
    m_processorToolbar->setVisible(false);
    m_processorTab = new ProcessorTab(m_controlToolbar, m_processorToolbar, this);
    m_stackedTabs->insertWidget(1, m_processorTab);

    m_memoryToolbar = addToolBar("Memory");
    m_memoryToolbar->setVisible(false);
    m_memoryTab = new MemoryTab(m_memoryToolbar, this);
    m_stackedTabs->insertWidget(2, m_memoryTab);

    // Setup tab bar
    m_ui->tabbar->addFancyTab(QIcon(":/icons/binary-code.svg"), "Editor");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/cpu.svg"), "Processor");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/ram-memory.svg"), "Memory");
    connect(m_stackedTabs, &QStackedWidget::currentChanged, this, &MainWindow::tabChanged);
    connect(m_ui->tabbar, &FancyTabBar::activeIndexChanged, m_stackedTabs, &QStackedWidget::setCurrentIndex);
    connect(m_ui->tabbar, &FancyTabBar::activeIndexChanged, m_editTab, &EditTab::updateProgramViewerHighlighting);

    setupMenus();

    // setup and connect widgets
    connect(m_processorTab, &ProcessorTab::update, this, &MainWindow::updateMemoryTab);
    connect(m_processorTab, &ProcessorTab::update, m_editTab, &EditTab::updateProgramViewerHighlighting);
    connect(this, &MainWindow::update, m_processorTab, &ProcessorTab::restart);
    connect(this, &MainWindow::updateMemoryTab, m_memoryTab, &MemoryTab::update);
    connect(m_stackedTabs, &QStackedWidget::currentChanged, m_memoryTab, &MemoryTab::update);
    connect(m_editTab, &EditTab::programChanged, ProcessorHandler::get(), &ProcessorHandler::loadProgram);
    connect(m_editTab, &EditTab::editorStateChanged, [=] { this->m_hasSavedFile = false; });

    connect(ProcessorHandler::get(), &ProcessorHandler::exit, m_processorTab, &ProcessorTab::processorFinished);
    connect(ProcessorHandler::get(), &ProcessorHandler::runFinished, m_processorTab, &ProcessorTab::runFinished);

    connect(&SystemIO::get(), &SystemIO::doPrint, m_processorTab, &ProcessorTab::printToLog);

    // Reset and program reload signals
    connect(m_memoryTab, &MemoryTab::reqProcessorReset, m_processorTab, &ProcessorTab::reset);
    connect(ProcessorHandler::get(), &ProcessorHandler::reqProcessorReset, m_processorTab, &ProcessorTab::reset);
    connect(ProcessorHandler::get(), &ProcessorHandler::reqReloadProgram, m_editTab, &EditTab::emitProgramChanged);
    connect(m_processorTab, &ProcessorTab::processorWasReset, [=] { SystemIO::reset(); });

    connect(m_ui->actionSystem_calls, &QAction::triggered, [=] {
        SyscallViewer v;
        v.exec();
    });
    connect(m_ui->actionOpen_wiki, &QAction::triggered, this, &MainWindow::wiki);
    connect(m_ui->actionVersion, &QAction::triggered, this, &MainWindow::version);
    connect(m_ui->actionSettings, &QAction::triggered, this, &MainWindow::settingsTriggered);

    m_ui->tabbar->setActiveIndex(1);
    m_processorToolbar->setVisible(true);
}

void MainWindow::tabChanged() {
    m_editToolbar->setVisible(m_editTab->isVisible());
    m_memoryToolbar->setVisible(m_memoryTab->isVisible());
    m_processorToolbar->setVisible(m_processorTab->isVisible());
}

void MainWindow::fitToView() {
    m_processorTab->fitToView();
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
    auto* assemblyMenu = parent->addMenu("Assembly");
    if (!assemblyExamples.isEmpty()) {
        for (const auto& fileName : assemblyExamples) {
            assemblyMenu->addAction(fileName, [=] {
                LoadFileParams parms;
                parms.filepath = QString(":/examples/assembly/") + fileName;
                parms.type = SourceType::Assembly;
                m_editTab->loadExternalFile(parms);
                m_hasSavedFile = false;
            });
        }
    }

    const auto cExamples = QDir(":/examples/C/").entryList(QDir::Files);
    auto* cMenu = parent->addMenu("C");
    if (!cExamples.isEmpty()) {
        for (const auto& fileName : cExamples) {
            cMenu->addAction(fileName, [=] {
                LoadFileParams parms;
                parms.filepath = QString(":/examples/C/") + fileName;
                parms.type = SourceType::C;
                m_editTab->loadExternalFile(parms);
                m_hasSavedFile = false;
            });
        }
    }

    const auto ELFExamples = QDir(":/examples/ELF/").entryList(QDir::Files);
    auto* elfMenu = parent->addMenu("ELF (precompiled C)");
    if (!ELFExamples.isEmpty()) {
        for (const auto& fileName : ELFExamples) {
            elfMenu->addAction(fileName, [=] {
                // ELFIO Cannot read directly from the bundled resource file, so copy the ELF file to a temporary file
                // before loading the program.
                QTemporaryFile* tmpELFFile = QTemporaryFile::createNativeFile(":/examples/ELF/" + fileName);
                if (!tmpELFFile->open()) {
                    QMessageBox::warning(this, "Error", "Could not create temporary ELF file");
                    return;
                }

                LoadFileParams parms;
                parms.filepath = tmpELFFile->fileName();
                parms.type = SourceType::ExternalELF;
                m_editTab->loadExternalFile(parms);
                m_hasSavedFile = false;
                tmpELFFile->remove();
            });
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_editTab->isEditorEnabled() && !m_editTab->getAssemblyText().isEmpty()) {
        QMessageBox saveMsgBox(this);
        saveMsgBox.setWindowTitle("Ripes");
        saveMsgBox.setText("Save current program before exiting?");
        saveMsgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        const auto result = saveMsgBox.exec();
        if (result == QMessageBox::Cancel) {
            // Dont exit
            event->ignore();
            return;
        } else if (result == QMessageBox::Yes) {
            saveFilesTriggered();
        }
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::loadFileTriggered() {
    m_processorTab->pause();
    LoadDialog diag;
    if (!diag.exec())
        return;

    m_editTab->loadExternalFile(diag.getParams());
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
        if (diag.exec()) {
            m_hasSavedFile = true;
        }
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

void MainWindow::settingsTriggered() {
    SettingsDialog diag;
    diag.exec();
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
