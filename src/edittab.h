#pragma once
#include <QFile>
#include <QWidget>

#include "ripestab.h"

namespace Ui {
class EditTab;
}

class EditTab : public RipesTab {
    Q_OBJECT

public:
    EditTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~EditTab() override;

    void setAssemblyText(const QString& text);
    void setDisassemblerText();
    void setInputMode(bool isAssembly);
    void setTimerEnabled(bool state);
    void clearOutputArray();
    QString getAssemblyText();
    const QByteArray& getBinaryData();
    void newProgram();

signals:
    void loadBinaryFile();
    void loadAssemblyFile();
    void updateSimulator();  // Emitted when a file has been successfully loaded or assembled, and binary info must be
                             // sent to the processor

private slots:
    void on_assemblyfile_toggled(bool checked);
    void assemblingComplete(const QByteArray& binaryCode, bool clear = true, uint32_t baseAddress = 0x0);

    void on_disassembledViewButton_toggled(bool checked);

private:
    Ui::EditTab* m_ui;
};
