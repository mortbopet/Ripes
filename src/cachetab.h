#ifndef CACHETAB_H
#define CACHETAB_H

#include <QWidget>
#include "runnercache.h"

namespace Ui {
class CacheTab;
}

class CacheSetupWidget;

class CacheTab : public QWidget {
    friend class RunnerCache;
    Q_OBJECT

public:
    explicit CacheTab(QWidget* parent = nullptr);
    ~CacheTab() override;

private:
    Ui::CacheTab* m_ui;

    void connectWidgets();
    void setupWidgets();
    RunnerCache* m_runnerCachePtr;
    void connectSetupWidget(CacheBase* cachePtr, cacheLevel level);

signals:
    void countChanged();
    void cacheChanged();

private slots:
    void cacheCountChanged(bool state);
};

#endif  // CACHETAB_H
