#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_actionexit_triggered();

  void on_actionLoadBinaryFile_triggered();

  void on_actionLoadAssemblyFile_triggered();

private:
  Ui::MainWindow *m_ui;
};

#endif // MAINWINDOW_H
