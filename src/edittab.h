#pragma once
#include <QByteArray>
#include <QFile>
#include <QWidget>
#include <map>

#include "assembler.h"
#include "ripestab.h"

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
    void updateSimulator();  // Emitted when a file has been successfully loaded or assembled, and binary info must be
                             // sent to the processor
    void programChanged(const std::map<uint32_t, QByteArray*> program);

public slots:
    void emitProgramChanged();

private slots:
    void assemble();
    void on_assemblyfile_toggled(bool checked);
    void on_disassembledViewButton_toggled(bool checked);

private:
    void assemblingComplete(const QByteArray& data, bool clear = true, uint32_t baseAddress = 0x0);

    Ui::EditTab* m_ui;
    Assembler* m_assembler = nullptr;
};
