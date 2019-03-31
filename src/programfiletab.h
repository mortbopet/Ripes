#ifndef PROGRAMFILETAB_H
#define PROGRAMFILETAB_H

#include <QFile>
#include <QWidget>

namespace Ui {
class ProgramfileTab;
}

class ProgramfileTab : public QWidget {
    Q_OBJECT

public:
    explicit ProgramfileTab(QWidget* parent = nullptr);
    ~ProgramfileTab() override;

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
    void on_pushButton_clicked();

    void on_assemblyfile_toggled(bool checked);
    void assemblingComplete(const QByteArray& binaryCode, bool clear = true, uint32_t baseAddress = 0x0);

    void on_disassembledViewButton_toggled(bool checked);

private:
    Ui::ProgramfileTab* m_ui;
};

#endif  // PROGRAMFILETAB_H
