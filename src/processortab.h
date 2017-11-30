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

    void initRegWidget(std::vector<uint32_t>* regPtr);
    void initInstructionView(memory* mem, Parser* parser, int textSize);

private slots:
    void on_expandView_clicked();

    void on_displayValues_toggled(bool checked);

private:
    Ui::ProcessorTab* m_ui;
    InstructionModel* m_instrModel;
};

#endif  // PROCESSORTAB_H
