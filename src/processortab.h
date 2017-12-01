#ifndef PROCESSORTAB_H
#define PROCESSORTAB_H

#include <QWidget>
#include "defines.h"

namespace Ui {
class ProcessorTab;
}

class InstructionModel;
class Parser;

class ProcessorTab : public QWidget {
    Q_OBJECT

public:
    explicit ProcessorTab(QWidget* parent = 0);
    ~ProcessorTab();

    void initRegWidget();
    void initInstructionView();

public slots:
    void update();

private slots:

    void on_expandView_clicked();

    void on_displayValues_toggled(bool checked);

    void on_run_clicked();

    void on_reset_clicked();

    void on_step_clicked();

private:
    Ui::ProcessorTab* m_ui;
    InstructionModel* m_instrModel;
};

#endif  // PROCESSORTAB_H
