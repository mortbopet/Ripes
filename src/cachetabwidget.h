#pragma once
#include <QWidget>

#include "cachesim/l1cacheshim.h"

namespace Ripes {
class CacheWidget;

namespace Ui {
class CacheTabWidget;
}

class CacheTabWidget : public QWidget {
    Q_OBJECT

public:
    explicit CacheTabWidget(QWidget* parent = nullptr);
    ~CacheTabWidget();

signals:
    void focusAddressChanged(unsigned address);

private:
    enum FixedCacheIdx { DataCache, InstrCache };
    void connectCacheWidget(CacheWidget* w);
    void handleTabIndexChanged(int index);
    void handleTabCloseRequest(int index);

    Ui::CacheTabWidget* m_ui;

    int m_addTabIdx = -1;
    int m_nextCacheLevel = 2;
    QSize m_defaultTabButtonSize;

    std::unique_ptr<L1CacheShim> m_l1dShim;
    std::unique_ptr<L1CacheShim> m_l1iShim;
};

}  // namespace Ripes
