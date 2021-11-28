#pragma once

#include <QMainWindow>

#include "assembler/program.h"
#include "statusmanager.h"

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
class CacheTab;
class IOTab;
class ProcessorHandler;
class RipesTab;
struct LoadFileParams;

struct TabWidgets {
    RipesTab* tab;
    QToolBar* toolbar;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

    enum TabIndex { EditTabID, ProcessorTabID, CacheTabID, MemoryTabID, IOTabID, NTabsID };

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void closeEvent(QCloseEvent* event) override;
    void fitToView();

signals:
    void prepareSave();

private slots:
    void wiki();
    void version();

    void loadFileTriggered();

    void saveFilesTriggered();
    void saveFilesAsTriggered();
    void newProgramTriggered();
    void settingsTriggered();
    void tabChanged(int index);

private:
    void loadFile(const QString& filename, SourceType type);

    void setupStatusBar();
    void setupMenus();
    void setupExamplesMenu(QMenu* parent);

    Ui::MainWindow* m_ui = nullptr;
    QActionGroup* m_binaryStoreAction;
    QToolBar* m_toolbar = nullptr;

    // Tabs
    QStackedWidget* m_stackedTabs = nullptr;
    std::map<TabIndex, TabWidgets> m_tabWidgets;
    TabIndex m_currentTabID = ProcessorTabID;
};
}  // namespace Ripes
