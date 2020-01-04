#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

QT_FORWARD_DECLARE_CLASS(QToolBar)
QT_FORWARD_DECLARE_CLASS(QStackedWidget)

class EditTab;
class MemoryTab;
class ProcessorTab;

QT_FORWARD_DECLARE_CLASS(QActionGroup)

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void loadBinaryFile(QString fileName);
    void loadAssemblyFile(QString fileName);

    void run();

private slots:
    void exit();
    void about();
    void wiki();

    void loadBinaryFileTriggered();
    void loadAssemblyFileTriggered();
    void saveFilesTriggered();
    void saveFilesAsTriggered();
    void newProgramTriggered();

    void processorUpdated() { emit updateMemoryTab(); }

    void tabChanged();

signals:
    void update();
    void updateMemoryTab();

private:
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
