#ifndef PROCESSORTAB_H
#define PROCESSORTAB_H

#include <QAction>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QWidget>

#include "defines.h"
// #include "pipelinetable.h"
// #include "pipelinetablemodel.h"
#include "ripestab.h"

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
    void rewind();
    void printToLog(const QString&);
    void processorFinished();

    void processorSelection();

private slots:
    void clock();
    void setInstructionViewCenterAddr(uint32_t address);
    // void showPipeliningTable();

private:
    void setupSimulatorActions();
    void enableSimulatorControls();
    void updateInstructionModel();
    void updateRegisterModel();

    Ui::ProcessorTab* m_ui;
    InstructionModel* m_instrModel = nullptr;

    vsrtl::VSRTLWidget* m_vsrtlWidget = nullptr;

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
};

#endif  // PROCESSORTAB_H
