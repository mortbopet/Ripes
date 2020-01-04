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
#include "ripestab.h"

namespace Ui {
class ProcessorTab;
}

class InstructionModel;
class Parser;

class ProcessorTab : public RipesTab {
    friend class RunDialog;
    Q_OBJECT

public:
    ProcessorTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~ProcessorTab() override;

    void initRegWidget();
    void initInstructionView();

public slots:
    void restart();
    void run();

private slots:
    void expandView();
    void displayValues(bool checked);
    void reset();
    void clock();
    void setCurrentInstruction(int row);
    void showPipeliningTable();
    void updateMetrics();

signals:
    void update();
    void appendToLog(QString string);

private:
    void setupSimulatorActions();
    void updateActionState();

    bool handleEcall(const std::pair<Pipeline::ECALL, int32_t>& ecallValue);

    Ui::ProcessorTab* m_ui;
    InstructionModel* m_instrModel;

    // Actions
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
