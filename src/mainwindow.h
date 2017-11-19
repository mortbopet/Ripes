#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class Runner;
class Parser;

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(Runner* runnerPtr, Parser* parserPtr,
                        QWidget* parent = 0);
    ~MainWindow();

   private slots:
    void on_actionexit_triggered();

    void on_actionLoadBinaryFile_triggered();
    void on_actionLoadAssemblyFile_triggered();
    void loadBinaryFile(QString fileName);
    void loadAssemblyFile(QString fileName);

    void on_actionAbout_triggered();

    void on_actionOpen_documentation_triggered();

   private:
    Ui::MainWindow* m_ui;
    void setupExamples();

    Runner* m_runnerPtr;
    Parser* m_parserPtr;
};

#endif  // MAINWINDOW_H
