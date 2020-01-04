#ifndef PROCESSORTAB_H
#define PROCESSORTAB_H

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
    void on_run_clicked();

private slots:
    void on_expandView_clicked();
    void on_displayValues_toggled(bool checked);
    void on_reset_clicked();
    void on_step_clicked();
    void toggleTimer(bool state);
    void on_zoomIn_clicked();
    void on_zoomOut_clicked();
    void on_save_clicked();
    void setCurrentInstruction(int row);

    void on_table_clicked();

    void updateMetrics();

private:
    bool handleEcall(const std::pair<Pipeline::ECALL, int32_t>& ecallValue);

    Ui::ProcessorTab* m_ui;
    InstructionModel* m_instrModel;

    QTimer m_timer;

signals:
    void update();
    void appendToLog(QString string);
};

#endif  // PROCESSORTAB_H
