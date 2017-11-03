#ifndef CACHETAB_H
#define CACHETAB_H

#include "runnercache.h"
#include <QWidget>

namespace Ui {
class CacheTab;
}

class CacheSetupWidget;

class CacheTab : public QWidget {
  friend class RunnerCache;
  Q_OBJECT

public:
  explicit CacheTab(QWidget *parent = 0);
  ~CacheTab();

  void setRunnerCachePtr(RunnerCache *ptr);

private:
  Ui::CacheTab *m_ui;

  void connectWidgets();
  void setupWidgets();
  RunnerCache *m_runnerCachePtr;
  void connectSetupWidget(CacheBase *cachePtr, cacheLevel level);

signals:
  void countChanged();
  void cacheChanged();

private slots:
  void cacheCountChanged(bool state);
};

#endif // CACHETAB_H
