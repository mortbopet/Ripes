#include "cacheconfigwidget.h"
#include "ui_cacheconfigwidget.h"

#include <QMessageBox>
#include <QSpinBox>

namespace Ripes {

CacheConfigWidget::CacheConfigWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheConfigWidget) {
    m_ui->setupUi(this);
}

void CacheConfigWidget::setCache(CacheSim* cache) {
    m_cache = cache;

    for (const auto& policy : s_cachePolicyStrings) {
        m_ui->replacementPolicy->addItem(policy.second, QVariant::fromValue(policy.first));
    }
    m_ui->ways->setValue(m_cache->getWaysBits());
    m_ui->lines->setValue(m_cache->getLineBits());
    m_ui->blocks->setValue(m_cache->getBlockBits());

    connect(m_ui->ways, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheSim::setWays);
    connect(m_ui->blocks, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheSim::setBlocks);
    connect(m_ui->lines, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheSim::setLines);
    connect(m_ui->sizeBreakdownButton, &QPushButton::clicked, this, &CacheConfigWidget::showSizeBreakdown);
    connect(m_ui->replacementPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        m_cache->setReplacementPolicy(qvariant_cast<CacheReplPlcy>(m_ui->replacementPolicy->itemData(index)));
    });

    setupPresets();
    m_ui->replacementPolicy->setCurrentIndex(0);
}

void CacheConfigWidget::setupPresets() {
    std::vector<std::pair<QString, CachePreset>> presets;

    presets.push_back({"32-entry 4-word direct-mapped", CachePreset{2, 5, 0}});
    presets.push_back({"32-entry 4-word fully associative", CachePreset{2, 0, 5}});
    presets.push_back({"32-entry 4-word 2-way set associative", CachePreset{2, 4, 1}});

    for (const auto& preset : presets) {
        m_ui->presets->addItem(preset.first, QVariant::fromValue<CachePreset>(preset.second));
    }

    connect(m_ui->presets, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        const CachePreset preset = qvariant_cast<CachePreset>(m_ui->presets->itemData(index));

        // Block signal emmision from the value widgets; the cache sim is updated all at once internally, so no need to
        // emit signals.
        m_ui->ways->blockSignals(true);
        m_ui->lines->blockSignals(true);
        m_ui->blocks->blockSignals(true);
        m_ui->ways->setValue(preset.ways);
        m_ui->lines->setValue(preset.lines);
        m_ui->blocks->setValue(preset.blocks);
        m_ui->ways->blockSignals(false);
        m_ui->lines->blockSignals(false);
        m_ui->blocks->blockSignals(false);

        m_cache->setPreset(preset);
    });

    m_ui->presets->setCurrentIndex(0);
}

void CacheConfigWidget::setCacheSize(unsigned size) {
    m_ui->size->setText(QString::number(size));
}

void CacheConfigWidget::setHitRate(double hitrate) {
    m_ui->hitrate->setText(QString::number(hitrate));
}

void CacheConfigWidget::showSizeBreakdown() {
    QString sizeText;

    auto cacheSize = m_cache->getCacheSize();

    for (const auto& component : cacheSize.components) {
        sizeText += component + "\n";
    }

    sizeText += "\nTotal: " + QString::number(cacheSize.bits) + " Bits";

    QMessageBox::information(this, "Cache size breakdown", sizeText);
}

CacheConfigWidget::~CacheConfigWidget() {
    delete m_ui;
}

}  // namespace Ripes
