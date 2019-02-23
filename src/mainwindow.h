#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void loadBinaryFile(QString fileName);
    void loadAssemblyFile(QString fileName);

private slots:
    void on_actionexit_triggered();

    void on_actionLoadBinaryFile_triggered();
    void on_actionLoadAssemblyFile_triggered();

    void on_actionAbout_triggered();

    void on_actionOpen_wiki_triggered();
    void processorUpdated() { emit updateMemoryTab(); }

    void on_actionSave_Files_triggered();

    void on_actionSave_Files_As_triggered();

signals:
    void update();
    void updateMemoryTab();

private:
    Ui::MainWindow* m_ui;
    void setupExamples();
    QString m_currentFile = QString();
};

#endif  // MAINWINDOW_H
