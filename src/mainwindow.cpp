#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "cachetab.h"
#include "edittab.h"
#include "iotab.h"
#include "loaddialog.h"
#include "memorytab.h"
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
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTemporaryFile>
#include <QTextStream>

namespace Ripes {

static void clearSaveFile() {
    RipesSettings::setValue(RIPES_SETTING_HAS_SAVEFILE, false);
    RipesSettings::setValue(RIPES_SETTING_SAVEPATH, "");
}

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

    auto* controlToolbar = addToolBar("Simulator control");
    controlToolbar->setVisible(true);  // Always visible

    auto* editToolbar = addToolBar("Edit");
    editToolbar->setVisible(false);
    auto* editTab = new EditTab(editToolbar, this);
    m_stackedTabs->insertWidget(EditTabID, editTab);
    m_tabWidgets[EditTabID] = {editTab, editToolbar};

    auto* processorToolbar = addToolBar("Processor");
    processorToolbar->setVisible(false);
    auto* processorTab = new ProcessorTab(controlToolbar, processorToolbar, this);
    m_stackedTabs->insertWidget(ProcessorTabID, processorTab);
    m_tabWidgets[ProcessorTabID] = {processorTab, processorToolbar};

    auto* cacheToolbar = addToolBar("Cache");
    cacheToolbar->setVisible(false);
    auto* cacheTab = new CacheTab(cacheToolbar, this);
    m_stackedTabs->insertWidget(CacheTabID, cacheTab);
    m_tabWidgets[CacheTabID] = {cacheTab, cacheToolbar};

    auto* memoryToolbar = addToolBar("Memory");
    memoryToolbar->setVisible(false);
    auto* memoryTab = new MemoryTab(memoryToolbar, this);
    m_stackedTabs->insertWidget(MemoryTabID, memoryTab);
    m_tabWidgets[MemoryTabID] = {memoryTab, memoryToolbar};

    auto* IOToolbar = addToolBar("I/O");
    IOToolbar->setVisible(false);
    auto* IOTab = new class IOTab(IOToolbar, this);
    m_stackedTabs->insertWidget(IOTabID, IOTab);
    m_tabWidgets[IOTabID] = {IOTab, IOToolbar};

    // Setup tab bar
    m_ui->tabbar->addFancyTab(QIcon(":/icons/binary-code.svg"), "Editor");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/cpu.svg"), "Processor");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/server.svg"), "Cache");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/ram-memory.svg"), "Memory");
    m_ui->tabbar->addFancyTab(QIcon(":/icons/led.svg"), "I/O");
    connect(m_ui->tabbar, &FancyTabBar::activeIndexChanged, this, &MainWindow::tabChanged);
    connect(m_ui->tabbar, &FancyTabBar::activeIndexChanged, m_stackedTabs, &QStackedWidget::setCurrentIndex);
    connect(m_ui->tabbar, &FancyTabBar::activeIndexChanged, editTab, &EditTab::updateProgramViewerHighlighting);
    setupMenus();

    // setup and connect widgets
    connect(editTab, &EditTab::editorStateChanged, [=] { clearSaveFile(); });

    // Setup status bar
    setupStatusBar();

    // Reset and program reload signals
    connect(ProcessorHandler::get(), &ProcessorHandler::processorReset, [=] { SystemIO::reset(); });

    connect(m_ui->actionSystem_calls, &QAction::triggered, [=] {
        SyscallViewer v;
        v.exec();
    });
    connect(m_ui->actionOpen_wiki, &QAction::triggered, this, &MainWindow::wiki);
    connect(m_ui->actionVersion, &QAction::triggered, this, &MainWindow::version);
    connect(m_ui->actionSettings, &QAction::triggered, this, &MainWindow::settingsTriggered);

    connect(cacheTab, &CacheTab::focusAddressChanged, memoryTab, &MemoryTab::setCentralAddress);

    connect(this, &MainWindow::prepareSave, editTab, &EditTab::onSave);

    m_currentTabID = ProcessorTabID;
    m_ui->tabbar->setActiveIndex(m_currentTabID);
}

#define _setupStatusWidget(name, _permanent)                                                                          \
    auto* name##StatusLabel = new QLabel(this);                                                                       \
    statusBar()->add##_permanent##Widget(name##StatusLabel);                                                          \
    connect(&name##StatusManager::get().emitter, &StatusEmitter::statusChanged, name##StatusLabel, &QLabel::setText); \
    connect(&name##StatusManager::get().emitter, &StatusEmitter::clear, name##StatusLabel, &QLabel::clear);

#define setupPermanentStatusWidget(name) \
    _setupStatusWidget(name, Permanent); \
    name##StatusManager::get().setPermanent();

#define setupStatusWidget(name) _setupStatusWidget(name, )

void MainWindow::setupStatusBar() {
    statusBar()->showMessage("");

    // Setup selected processor & ISA info status widget (right-aligned => permanent)
    setupPermanentStatusWidget(ProcessorInfo);
    auto updateProcessorInfo = [=] {
        const auto& desc = ProcessorRegistry::getDescription(ProcessorHandler::getID());
        QString status =
            "Processor: " + desc.name + "    ISA: " + ProcessorHandler::getProcessor()->implementsISA()->name();
        ProcessorInfoStatusManager::get().setStatusPermanent(status);
    };
    connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, updateProcessorInfo);
    updateProcessorInfo();

    // Setup processorhandler status widget
    setupStatusWidget(Processor);

    // Setup syscall status widget
    setupStatusWidget(Syscall);

    // Setup systemIO status widget
    setupStatusWidget(SystemIO);

    // Setup general info status widget
    setupStatusWidget(General);
}

void MainWindow::tabChanged(int index) {
    m_tabWidgets.at(m_currentTabID).toolbar->setVisible(false);
    m_tabWidgets.at(m_currentTabID).tab->tabVisibilityChanged(false);
    m_currentTabID = static_cast<TabIndex>(index);
    m_tabWidgets.at(m_currentTabID).toolbar->setVisible(true);
    m_tabWidgets.at(m_currentTabID).tab->tabVisibilityChanged(true);
}

void MainWindow::fitToView() {
    static_cast<ProcessorTab*>(m_tabWidgets.at(ProcessorTabID).tab)->fitToScreen();
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
    connect(static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab), &EditTab::editorStateChanged,
            [saveAction](bool enabled) { saveAction->setEnabled(enabled); });
    m_ui->menuFile->addAction(saveAction);

    const QIcon saveAsIcon = QIcon(":/icons/saveas.svg");
    auto* saveAsAction = new QAction(saveAsIcon, "Save File As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFilesAsTriggered);
    connect(static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab), &EditTab::editorStateChanged,
            [saveAsAction](bool enabled) { saveAsAction->setEnabled(enabled); });
    m_ui->menuFile->addAction(saveAsAction);

    m_ui->menuFile->addSeparator();

    const QIcon exitIcon = QIcon(":/icons/cancel.svg");
    auto* exitAction = new QAction(exitIcon, "Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    m_ui->menuFile->addAction(exitAction);

    m_ui->menuView->addAction(static_cast<ProcessorTab*>(m_tabWidgets.at(ProcessorTabID).tab)->m_darkmodeAction);
    m_ui->menuView->addAction(static_cast<ProcessorTab*>(m_tabWidgets.at(ProcessorTabID).tab)->m_displayValuesAction);
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::setupExamplesMenu(QMenu* parent) {
    const auto assemblyExamples = QDir(":/examples/assembly/").entryList(QDir::Files);
    auto* assemblyMenu = parent->addMenu("Assembly");
    if (!assemblyExamples.isEmpty()) {
        for (const auto& fileName : assemblyExamples) {
            assemblyMenu->addAction(fileName, this, [=] {
                LoadFileParams parms;
                parms.filepath = QString(":/examples/assembly/") + fileName;
                parms.type = SourceType::Assembly;
                static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->loadExternalFile(parms);
                clearSaveFile();
            });
        }
    }

    const auto cExamples = QDir(":/examples/C/").entryList(QDir::Files);
    auto* cMenu = parent->addMenu("C");
    if (!cExamples.isEmpty()) {
        for (const auto& fileName : cExamples) {
            cMenu->addAction(fileName, this, [=] {
                LoadFileParams parms;
                parms.filepath = QString(":/examples/C/") + fileName;
                parms.type = SourceType::C;
                static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->loadExternalFile(parms);
                clearSaveFile();
            });
        }
    }

    const auto ELFExamples = QDir(":/examples/ELF/").entryList(QDir::Files);
    auto* elfMenu = parent->addMenu("ELF (precompiled C)");
    if (!ELFExamples.isEmpty()) {
        for (const auto& fileName : ELFExamples) {
            elfMenu->addAction(fileName, this, [=] {
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
                static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->loadExternalFile(parms);
                clearSaveFile();
                tmpELFFile->remove();
            });
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->isEditorEnabled() &&
        !static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->getAssemblyText().isEmpty()) {
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

    // Emit an observable signal to indicate that the application is about to close
    RipesSettings::setValue(RIPES_GLOBALSIGNAL_QUIT, 0);
    QMainWindow::closeEvent(event);
}

void MainWindow::loadFileTriggered() {
    static_cast<ProcessorTab*>(m_tabWidgets.at(ProcessorTabID).tab)->pause();
    LoadDialog diag;
    if (!diag.exec())
        return;

    if (static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->loadExternalFile(diag.getParams())) {
        // Set the current save path to the loaded file. Only store the file basename without extension.
        QFileInfo info = QFileInfo(diag.getParams().filepath);
        auto savePath = info.dir().path() + QDir::separator() + info.baseName();
        RipesSettings::setValue(RIPES_SETTING_SAVEPATH, savePath);
        RipesSettings::setValue(RIPES_SETTING_HAS_SAVEFILE, true);
    }
}

void MainWindow::wiki() {
    QDesktopServices::openUrl(QUrl(QString("https://github.com/mortbopet/Ripes/wiki")));
}

void MainWindow::version() {
    QMessageBox aboutDialog(this);
    aboutDialog.setText("Ripes version: " + getRipesVersion());
    aboutDialog.exec();
}

static void writeTextFile(QFile& file, const QString& data) {
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << data;
        file.close();
    }
}

static void writeBinaryFile(QFile& file, const QByteArray& data) {
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
    }
}

void MainWindow::saveFilesTriggered() {
    SaveDialog diag(static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->getSourceType());
    if (!RipesSettings::value(RIPES_SETTING_HAS_SAVEFILE).toBool()) {
        saveFilesAsTriggered();
        return;
    }

    emit prepareSave();

    bool didSave = false;
    QStringList savedFiles;

    if (!diag.sourcePath().isEmpty()) {
        QFile file(diag.sourcePath());
        savedFiles << diag.sourcePath();
        writeTextFile(file, static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->getAssemblyText());
        didSave |= true;
    }

    if (!diag.binaryPath().isEmpty()) {
        QFile file(diag.binaryPath());
        auto program = ProcessorHandler::getProgram();
        if (program && (program.get()->sections.count(".text") != 0)) {
            savedFiles << diag.binaryPath();
            writeBinaryFile(file, program.get()->sections.at(".text").data);
            didSave |= true;
        }
    }

    if (didSave) {
        GeneralStatusManager::setStatusTimed("Saved files " + savedFiles.join(", "), 1000);
    }
}

void MainWindow::saveFilesAsTriggered() {
    SaveDialog diag(static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->getSourceType());
    auto ret = diag.exec();
    if (ret == QDialog::Rejected) {
        return;
    }
    RipesSettings::setValue(RIPES_SETTING_HAS_SAVEFILE, true);
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
    if (!static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->getAssemblyText().isEmpty() ||
        RipesSettings::value(RIPES_SETTING_HAS_SAVEFILE).toBool()) {
        // User wrote a program but did not save it to a file yet
        mbox.setText("Save program before creating new file?");
        auto ret = mbox.exec();
        switch (ret) {
            case QMessageBox::Yes: {
                saveFilesTriggered();
                if (!RipesSettings::value(RIPES_SETTING_HAS_SAVEFILE).toBool()) {
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
    clearSaveFile();
    static_cast<EditTab*>(m_tabWidgets.at(EditTabID).tab)->newProgram();
}

}  // namespace Ripes
