#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

QT_FORWARD_DECLARE_CLASS(QActionGroup)

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void loadBinaryFile(QString fileName);
    void loadAssemblyFile(QString fileName);

    void run();

private slots:
    void on_actionexit_triggered();

    void on_actionLoadBinaryFile_triggered();
    void on_actionLoadAssemblyFile_triggered();

    void on_actionAbout_triggered();

    void on_actionOpen_wiki_triggered();
    void processorUpdated() { emit updateMemoryTab(); }

    void on_actionSave_Files_triggered();

    void on_actionSave_Files_As_triggered();

    void on_actionNew_Program_triggered();

signals:
    void update();
    void updateMemoryTab();

private:
    Ui::MainWindow* m_ui;
    void setupExamples();
    QString m_currentFile = QString();
    QActionGroup* m_binaryStoreAction;
};

#endif  // MAINWINDOW_H
