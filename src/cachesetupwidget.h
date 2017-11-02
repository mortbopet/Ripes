#ifndef CACHESETUPWIDGET_H
#define CACHESETUPWIDGET_H

#include <QWidget>

namespace Ui {
class CacheSetupWidget;
}

class CacheSetupWidget : public QWidget {
  Q_OBJECT

public:
  explicit CacheSetupWidget(QWidget *parent = 0);
  ~CacheSetupWidget();

  void setName(QString name);

signals:
  void groupBoxToggled(bool state);

public slots:
  void enable(bool state);

private slots:
  void on_groupbox_toggled(bool arg1);

private:
  Ui::CacheSetupWidget *m_ui;
};

#endif // CACHESETUPWIDGET_H
