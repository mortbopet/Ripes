#pragma once

#include <QAction>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QWidget>

#include "defines.h"
#include "ripestab.h"

namespace vsrtl {
class VSRTLWidget;
class Label;
}  // namespace vsrtl

namespace Ripes {

namespace Ui {
class ProcessorTab;
}

class InstructionModel;
class RegisterModel;
class StageTableModel;
struct Layout;

class ProcessorTab : public RipesTab {
    friend class RunDialog;
    Q_OBJECT

public:
    ProcessorTab(QToolBar* controlToolbar, QToolBar* additionalToolbar, QWidget* parent = nullptr);
    ~ProcessorTab() override;

    void initRegWidget();

signals:
    void update();
    void processorWasReset();

public slots:
    void pause();
    void restart();
    void reset();
    void reverse();
    void printToLog(const QString&);
    void processorFinished();
    void runFinished();
    void updateStatistics();
    void updateInstructionLabels();
    void fitToView();

    void processorSelection();

private slots:
    void run(bool state);
    void clock();
    void setInstructionViewCenterAddr(uint32_t address);
    void showStageTable();

private:
    void setupSimulatorActions(QToolBar* controlToolbar);
    void enableSimulatorControls();
    void updateInstructionModel();
    void updateRegisterModel();
    void loadLayout(const Layout&);
    void loadProcessorToWidget(const Layout&);

    Ui::ProcessorTab* m_ui = nullptr;
    InstructionModel* m_instrModel = nullptr;
    StageTableModel* m_stageModel = nullptr;

    vsrtl::VSRTLWidget* m_vsrtlWidget = nullptr;

    std::map<unsigned, vsrtl::Label*> m_stageInstructionLabels;

    QTimer* m_statUpdateTimer;

    // Actions
    QAction* m_selectProcessorAction = nullptr;
    QAction* m_clockAction = nullptr;
    QAction* m_autoClockAction = nullptr;
    QAction* m_runAction = nullptr;
    QAction* m_displayValuesAction = nullptr;
    QAction* m_stageTableAction = nullptr;
    QAction* m_reverseAction = nullptr;
    QAction* m_resetAction = nullptr;

    QSpinBox* m_autoClockInterval = nullptr;

    /**
     * @brief m_hasRun
     * True whenever the processor has been executed through the "Run" action.
     */
    bool m_hasRun = false;
};
}  // namespace Ripes
