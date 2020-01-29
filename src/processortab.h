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
    ProcessorTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~ProcessorTab() override;

    void initRegWidget();

signals:
    void update();
    void appendToLog(QString string);

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

    void processorSelection();

private slots:
    void run(bool state);
    void clock();
    void setInstructionViewCenterAddr(uint32_t address);
    void showStageTable();

private:
    void setupSimulatorActions();
    void enableSimulatorControls();
    void updateInstructionModel();
    void updateRegisterModel();
    void loadLayout(const Layout&);
    void loadProcessorToWidget(const Layout&);

    Ui::ProcessorTab* m_ui;
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
    QAction* m_pipelineTableAction = nullptr;
    QAction* m_reverseAction = nullptr;
    QAction* m_resetAction = nullptr;

    QSpinBox* m_autoClockInterval = nullptr;
};
}  // namespace Ripes
