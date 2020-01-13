#pragma once

#include <QByteArray>
#include <QFile>
#include <QWidget>
#include <map>

#include "assembler.h"
#include "program.h"
#include "ripestab.h"

namespace Ripes {

namespace Ui {
class EditTab;
}

class Assembler;

class EditTab : public RipesTab {
    Q_OBJECT

public:
    EditTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~EditTab() override;

    void setAssemblyText(const QString& text);
    void setDisassemblerText();
    void setInputMode(bool isAssembly);
    QString getAssemblyText();
    void newProgram();
    void clear();

    const QByteArray& getBinaryData();

signals:
    void programChanged(const Program& program);

public slots:
    void emitProgramChanged();

private slots:
    void assemble();
    void on_assemblyfile_toggled(bool checked);
    void on_disassembledViewButton_toggled(bool checked);

private:
    Ui::EditTab* m_ui;
    Assembler* m_assembler = nullptr;
};
}  // namespace Ripes
