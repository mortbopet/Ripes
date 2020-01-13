#pragma once

#include <QMainWindow>

#include "program.h"

QT_FORWARD_DECLARE_CLASS(QToolBar)
QT_FORWARD_DECLARE_CLASS(QStackedWidget)
QT_FORWARD_DECLARE_CLASS(QActionGroup)

namespace Ripes {

namespace Ui {
class MainWindow;
}

class EditTab;
class MemoryTab;
class ProcessorTab;
class ProcessorHandler;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void loadBinaryFile(QString fileName = QString());
    void loadAssemblyFile(QString fileName = QString());

    void run();

private slots:
    void exit();
    void about();
    void wiki();

    void loadFileTriggered();

    void saveFilesTriggered();
    void saveFilesAsTriggered();
    void newProgramTriggered();

    void processorUpdated() { emit updateMemoryTab(); }

    void tabChanged();

signals:
    void update();
    void updateMemoryTab();

private:
    void loadFile(const QString& filename, FileType type);

    void setupMenus();

    Ui::MainWindow* m_ui;
    void setupExamplesMenu(QMenu* parent);
    QString m_currentFile = QString();
    QActionGroup* m_binaryStoreAction;
    QToolBar* m_toolbar = nullptr;

    // Tabs
    QStackedWidget* m_stackedTabs = nullptr;
    ProcessorTab* m_processorTab = nullptr;
    EditTab* m_editTab = nullptr;
    MemoryTab* m_memoryTab = nullptr;
};
}  // namespace Ripes
