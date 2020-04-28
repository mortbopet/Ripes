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

    /**
     * @brief m_justSetPreset
     * Small helper flag for not erasing the text displaying the currently set preset. When false, this ensures that the
     * preset combo box is cleared when the user makes changes to the current cache configuration; such that we do not
     * give the impression that the preset is still in effect after a configuration change.
     */
    bool m_justSetPreset = false;
};

}  // namespace Ripes

// Qt Metatypes for enum combo boxes
Q_DECLARE_METATYPE(Ripes::CacheSim::WritePolicy);
Q_DECLARE_METATYPE(Ripes::CacheSim::WriteAllocPolicy);
Q_DECLARE_METATYPE(Ripes::CacheSim::ReplPolicy);
Q_DECLARE_METATYPE(Ripes::CacheSim::CachePreset);
