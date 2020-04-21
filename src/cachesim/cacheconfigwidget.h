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

signals:
    void configurationChanged();

public slots:
    void updateHitrate();
    void handleConfigurationChanged();
    void showCachePlot();

private:
    void updateCacheSize();
    void updateIndexingText();
    void setupPresets();
    void showSizeBreakdown();
    CacheSim* m_cache;
    Ui::CacheConfigWidget* m_ui = nullptr;
    std::vector<QObject*> m_configItems;
};

}  // namespace Ripes

// Qt Metatypes for enum combo boxes
Q_DECLARE_METATYPE(Ripes::CacheSim::WritePolicy);
Q_DECLARE_METATYPE(Ripes::CacheSim::WriteAllocPolicy);
Q_DECLARE_METATYPE(Ripes::CacheSim::ReplPolicy);
Q_DECLARE_METATYPE(Ripes::CacheSim::CachePreset);
