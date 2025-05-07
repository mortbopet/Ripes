#include "cacheconfigwidget.h"
#include "ui_cacheconfigwidget.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QtCharts/QChartView>

#include "cacheplotwidget.h"
#include "enumcombobox.h"
#include "ripessettings.h"

namespace Ripes {

CacheConfigWidget::CacheConfigWidget(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::CacheConfigWidget) {
  m_ui->setupUi(this);

  // Gather a list of all items in this widget which will trigger a modification
  // to the current configuration
  m_configItems = {
      m_ui->presets,           m_ui->ways,   m_ui->lines, m_ui->blocks,
      m_ui->replacementPolicy, m_ui->wrMiss, m_ui->wrHit};
}

void CacheConfigWidget::setCache(const std::shared_ptr<CacheSim> &cache) {
  m_cache = cache;

  setupEnumCombobox(m_ui->replacementPolicy, s_cacheReplPolicyStrings);
  setupEnumCombobox(m_ui->wrHit, s_cacheWritePolicyStrings);
  setupEnumCombobox(m_ui->wrMiss, s_cacheWriteAllocateStrings);

  m_ui->ways->setValue(m_cache->getWays());
  m_ui->lines->setValue(m_cache->getLineBits());
  m_ui->blocks->setValue(m_cache->getBlockBits());

  connect(m_ui->ways, QOverload<int>::of(&QSpinBox::valueChanged),
          m_cache.get(), &CacheSim::setWays);
  connect(m_ui->blocks, QOverload<int>::of(&QSpinBox::valueChanged),
          m_cache.get(), &CacheSim::setBlocks);
  connect(m_ui->lines, QOverload<int>::of(&QSpinBox::valueChanged),
          m_cache.get(), &CacheSim::setLines);

  connect(m_ui->replacementPolicy,
          QOverload<int>::of(&QComboBox::currentIndexChanged), cache.get(),
          [=](int index) {
            m_cache->setReplacementPolicy(qvariant_cast<ReplPolicy>(
                m_ui->replacementPolicy->itemData(index)));
          });
  connect(m_ui->wrHit, QOverload<int>::of(&QComboBox::currentIndexChanged),
          cache.get(), [=](int index) {
            m_cache->setWritePolicy(
                qvariant_cast<WritePolicy>(m_ui->wrHit->itemData(index)));
          });
  connect(m_ui->wrMiss, QOverload<int>::of(&QComboBox::currentIndexChanged),
          cache.get(), [=](int index) {
            m_cache->setWriteAllocatePolicy(
                qvariant_cast<WriteAllocPolicy>(m_ui->wrMiss->itemData(index)));
          });
  connect(m_ui->savePresetButton, &QPushButton::clicked, this,
          &CacheConfigWidget::storePreset);
  m_ui->savePresetButton->setIcon(QIcon(":/icons/save.svg"));
  m_ui->savePresetButton->setToolTip("Store cache preset");
  connect(m_ui->removePresetButton, &QPushButton::clicked, this,
          &CacheConfigWidget::removePreset);
  m_ui->removePresetButton->setIcon(QIcon(":/icons/delete.svg"));
  m_ui->removePresetButton->setToolTip("Delete cache preset");

  connect(m_cache.get(), &CacheSim::configurationChanged, this,
          &CacheConfigWidget::handleConfigurationChanged);
  connect(m_cache.get(), &CacheSim::configurationChanged, this,
          [=] { emit configurationChanged(); });

  setupPresets();
  handleConfigurationChanged();
}

void CacheConfigWidget::storePreset() {
  bool ok;
  QString text = QInputDialog::getText(
      this, "Store cache preset", "Preset name:", QLineEdit::Normal, "", &ok);
  if (ok && !text.isEmpty()) {
    CachePreset preset;
    preset.name = text;
    preset.ways = m_ui->ways->value();
    preset.lines = m_ui->lines->value();
    preset.blocks = m_ui->blocks->value();
    preset.wrPolicy = getEnumValue<WritePolicy>(m_ui->wrHit);
    preset.wrAllocPolicy = getEnumValue<WriteAllocPolicy>(m_ui->wrMiss);
    preset.replPolicy = getEnumValue<ReplPolicy>(m_ui->replacementPolicy);

    auto presets = RipesSettings::value(RIPES_SETTING_CACHE_PRESETS)
                       .value<QList<Ripes::CachePreset>>();
    presets.push_back(preset);
    RipesSettings::setValue(RIPES_SETTING_CACHE_PRESETS,
                            QVariant::fromValue(presets));

    m_ui->presets->addItem(preset.name,
                           QVariant::fromValue<CachePreset>(preset));
    m_ui->presets->setCurrentIndex(m_ui->presets->count() - 1);
  }
}

void CacheConfigWidget::removePreset() {
  auto presetData = m_ui->presets->currentData();
  if (presetData.isNull()) {
    return;
  }

  auto preset = presetData.value<CachePreset>();
  const QString prompt =
      "Are you sure you want to delete preset '" + preset.name + "'?";
  auto button = QMessageBox::information(this, "Delete cache preset", prompt,
                                         QMessageBox::No | QMessageBox::Yes);
  if (button != QMessageBox::Yes) {
    return;
  }

  auto presets = RipesSettings::value(RIPES_SETTING_CACHE_PRESETS)
                     .value<QList<Ripes::CachePreset>>();
  presets.removeOne(preset);
  RipesSettings::setValue(RIPES_SETTING_CACHE_PRESETS,
                          QVariant::fromValue(presets));
  // delete preset, but keep preset data by setting an invalid index (avoids
  // loading the next preset in the combobox)
  const auto idxToDelete = m_ui->presets->currentIndex();
  m_ui->presets->setCurrentIndex(-1);
  m_ui->presets->removeItem(idxToDelete);
}

void CacheConfigWidget::setupPresets() {
  for (const auto &preset : RipesSettings::value(RIPES_SETTING_CACHE_PRESETS)
                                .value<QList<CachePreset>>()) {
    m_ui->presets->addItem(preset.name,
                           QVariant::fromValue<CachePreset>(preset));
  }

  connect(m_ui->presets, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [=](int index) {
            if (index == -1) {
              return;
            }
            const CachePreset _preset =
                qvariant_cast<CachePreset>(m_ui->presets->itemData(index));
            m_justSetPreset = true;
            m_cache->setPreset(_preset);
          });
}

void CacheConfigWidget::handleConfigurationChanged() {
  std::for_each(m_configItems.begin(), m_configItems.end(),
                [](QObject *o) { o->blockSignals(true); });

  m_ui->ways->setValue(m_cache->getWays());
  m_ui->lines->setValue(m_cache->getLineBits());
  m_ui->blocks->setValue(m_cache->getBlockBits());
  setEnumIndex(m_ui->wrHit, m_cache->getWritePolicy());
  setEnumIndex(m_ui->wrMiss, m_cache->getWriteAllocPolicy());
  setEnumIndex(m_ui->replacementPolicy, m_cache->getReplacementPolicy());

  if (!m_justSetPreset) {
    m_ui->presets->setCurrentIndex(-1);
  }
  m_justSetPreset = false;

  std::for_each(m_configItems.begin(), m_configItems.end(),
                [](QObject *o) { o->blockSignals(false); });
}

CacheConfigWidget::~CacheConfigWidget() { delete m_ui; }

} // namespace Ripes
