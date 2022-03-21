#pragma once

#include "cachesim.h"
#include <QWidget>

namespace Ripes {

namespace Ui {
class CacheConfigWidget;
}

class CacheConfigWidget : public QWidget {
  Q_OBJECT

public:
  CacheConfigWidget(QWidget *parent);
  ~CacheConfigWidget() override;

  void setCache(const std::shared_ptr<CacheSim> &cache);

signals:
  void configurationChanged();

public slots:
  void handleConfigurationChanged();

private:
  void setupPresets();
  void showSizeBreakdown();
  std::shared_ptr<CacheSim> m_cache;
  Ui::CacheConfigWidget *m_ui = nullptr;
  std::vector<QObject *> m_configItems;
  void storePreset();
  void removePreset();

  /**
   * @brief m_justSetPreset
   * Small helper flag for not erasing the text displaying the currently set
   * preset. When false, this ensures that the preset combo box is cleared when
   * the user makes changes to the current cache configuration; such that we do
   * not give the impression that the preset is still in effect after a
   * configuration change.
   */
  bool m_justSetPreset = false;
};

} // namespace Ripes

// Qt Metatypes for enum combo boxes
Q_DECLARE_METATYPE(Ripes::WritePolicy);
Q_DECLARE_METATYPE(Ripes::WriteAllocPolicy);
Q_DECLARE_METATYPE(Ripes::ReplPolicy);
Q_DECLARE_METATYPE(Ripes::CachePreset);
