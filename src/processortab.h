#ifndef PROCESSORTAB_H
#define PROCESSORTAB_H

#include <QAction>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QWidget>

#include "defines.h"
#include "pipeline.h"
#include "pipelinetable.h"
#include "pipelinetablemodel.h"
#include "processorhandler.h"
#include "ripestab.h"

#include "graphics/pipelinewidget.h"

namespace Ui {
class ProcessorTab;
}

namespace vsrtl {
class VSRTLWidget;
}

class InstructionModel;
class RegisterModel;

class ProcessorTab : public RipesTab {
    friend class RunDialog;
    Q_OBJECT

public:
    ProcessorTab(ProcessorHandler& handler, QToolBar* toolbar, QWidget* parent = nullptr);
    ~ProcessorTab() override;

    void initRegWidget();

signals:
    void update();
    void appendToLog(QString string);

public slots:
    void pause();
    void restart();
    void reset();
    void rewind();
    void run();
    void printToLog(const QString&);
    void processorFinished();

    void processorSelection();

private slots:
    void expandView();
    void clock();
    void setInstructionViewCenterAddr(uint32_t address);
    void showPipeliningTable();
    void updateMetrics();

private:
    void setupSimulatorActions();
    void enableSimulatorControls();
    void updateInstructionModel();
    void updateRegisterModel();

    bool handleEcall(const std::pair<Pipeline::ECALL, int32_t>& ecallValue);

    Ui::ProcessorTab* m_ui;
    InstructionModel* m_instrModel = nullptr;

    vsrtl::VSRTLWidget* m_vsrtlWidget = nullptr;
    ProcessorHandler& m_handler;

    // Actions
    QAction* m_selectProcessorAction = nullptr;
    QAction* m_clockAction = nullptr;
    QAction* m_autoClockAction = nullptr;
    QAction* m_runAction = nullptr;
    QAction* m_displayValuesAction = nullptr;
    QAction* m_fitViewAction = nullptr;
    QAction* m_pipelineTableAction = nullptr;
    QAction* m_reverseAction = nullptr;
    QAction* m_resetAction = nullptr;

    QSpinBox* m_autoClockInterval = nullptr;

    PipelineWidget* tmp_pipelineWidget = nullptr;
};

#endif  // PROCESSORTAB_H
