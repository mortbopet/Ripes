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
    explicit ProgramfileTab(QWidget* parent = 0);
    ~ProgramfileTab();

    void setAssemblyText(const QString& text);
    void setDisassemblerText(const QString& text);
    void setInputMode(bool isAssembly);
    void setTimerEnabled(bool state);

signals:
    void loadBinaryFile();
    void loadAssemblyFile();
    void updateSimulator();  // Emitted when a file has been successfully loaded or assembled, and binary info must be
                             // sent to the processor

private slots:
    void on_pushButton_clicked();

    void on_assemblyfile_toggled(bool checked);
    void assemblingComplete(const QByteArray& binaryCode);

private:
    Ui::ProgramfileTab* m_ui;
};

#endif  // PROGRAMFILETAB_H
