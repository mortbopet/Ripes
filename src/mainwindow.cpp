#include "mainwindow.h"
#include "QFileDialog"
#include "ui_mainwindow.h"

#include "programfiletab.h"
#include "registerwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow) {
  m_ui->setupUi(this);
  setWindowTitle("RISC-V simulator");

  // setup icons
  /*
  m_ui->actionRun->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
  m_ui->actionStep->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_MediaSkipForward));
  m_ui->actionReset->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
  m_ui->actionDump->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_FileLinkIcon));
      */

  // connect widgets
  connect(m_ui->programfiletab, &ProgramfileTab::loadBinaryFile, this,
          &MainWindow::on_actionLoadBinaryFile_triggered);
  connect(m_ui->programfiletab, &ProgramfileTab::loadAssemblyFile, this,
          &MainWindow::on_actionLoadAssemblyFile_triggered);
}

MainWindow::~MainWindow() { delete m_ui; }

void MainWindow::on_actionexit_triggered() { close(); }

void MainWindow::on_actionLoadBinaryFile_triggered() {
  auto filename = QFileDialog::getOpenFileName(this, "Open binary file", "",
                                               "Binary file (*)");
}

void MainWindow::on_actionLoadAssemblyFile_triggered() {
  auto filename = QFileDialog::getOpenFileName(this, "Open assembly file", "",
                                               "Assembly file (*.as *.asm)");
}
