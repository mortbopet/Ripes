#ifndef PROGRAMFILETAB_H
#define PROGRAMFILETAB_H

#include <QWidget>

namespace Ui {
class ProgramfileTab;
}

class ProgramfileTab : public QWidget {
  Q_OBJECT

public:
  explicit ProgramfileTab(QWidget *parent = 0);
  ~ProgramfileTab();

signals:
  void loadBinaryFile();
  void loadAssemblyFile();

private slots:
  void on_pushButton_clicked();

private:
  Ui::ProgramfileTab *m_ui;
};

#endif // PROGRAMFILETAB_H
