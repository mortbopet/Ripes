#ifndef CACHETAB_H
#define CACHETAB_H

#include <QWidget>

namespace Ui {
class CacheTab;
}

class CacheTab : public QWidget {
  Q_OBJECT

public:
  explicit CacheTab(QWidget *parent = 0);
  ~CacheTab();

private:
  Ui::CacheTab *m_ui;

  void connectWidgets();

  void setupWidgets();

signals:
  void countChanged();
  void cacheChanged();

private slots:
  void cacheCountChanged(bool state);
};

#endif // CACHETAB_H
