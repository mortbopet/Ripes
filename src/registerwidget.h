#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget {
  Q_OBJECT

public:
  explicit RegisterWidget(QWidget *parent = 0);
  ~RegisterWidget();

  void setAlias(QString text);
  void setNumber(int number);

private:
  Ui::RegisterWidget *m_ui;
};

#endif // REGISTERWIDGET_H
