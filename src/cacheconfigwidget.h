#pragma once

#include <QWidget>
#include "cachesim.h"

namespace Ripes {

namespace Ui {
class CacheConfigWidget;
}

class CacheConfigWidget : public QWidget {
    Q_OBJECT

public:
    CacheConfigWidget(QWidget* parent);
    ~CacheConfigWidget() override;

    void setCache(CacheSim* cache);

public slots:
    void setHitRate(double hitrate);
    void configChanged();

private:
    void updateCacheSize();
    void updateIndexingText();
    void setupPresets();
    void showSizeBreakdown();
    CacheSim* m_cache;
    Ui::CacheConfigWidget* m_ui = nullptr;
};

}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::CacheSim::ReplPolicy);
Q_DECLARE_METATYPE(Ripes::CacheSim::CachePreset);
