#include "cacheconfigwidget.h"
#include "ui_cacheconfigwidget.h"

#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QtCharts/QChartView>

#include "cacheplotwidget.h"
#include "enumcombobox.h"

namespace Ripes {

CacheConfigWidget::CacheConfigWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheConfigWidget) {
    m_ui->setupUi(this);

    // Gather a list of all items in this widget which will trigger a modification to the current configuration
    m_configItems = {m_ui->presets,           m_ui->ways,   m_ui->lines, m_ui->blocks,
                     m_ui->replacementPolicy, m_ui->wrMiss, m_ui->wrHit};
}

void CacheConfigWidget::setCache(CacheSim* cache) {
    m_cache = cache;

    const QIcon sizeBreakdownIcon = QIcon(":/icons/info.svg");
    m_ui->sizeBreakdownButton->setIcon(sizeBreakdownIcon);

    const QIcon plotIcon = QIcon(":/icons/analytics.svg");
    m_ui->cachePlot->setIcon(plotIcon);
    connect(m_ui->cachePlot, &QPushButton::clicked, this, &CacheConfigWidget::showCachePlot);

    setupEnumCombobox(m_ui->replacementPolicy, s_cacheReplPolicyStrings);
    setupEnumCombobox(m_ui->wrHit, s_cacheWritePolicyStrings);
    setupEnumCombobox(m_ui->wrMiss, s_cacheWriteAllocateStrings);

    m_ui->ways->setValue(m_cache->getWaysBits());
    m_ui->lines->setValue(m_cache->getLineBits());
    m_ui->blocks->setValue(m_cache->getBlockBits());

    m_ui->indexingKey->setText(
        "<font color=\"gray\">█</font> = Tag &nbsp; <font color=\"red\">█</font> = Index &nbsp; <font "
        "color=\"green\">█</font> = Block &nbsp; <font color=\"black\">█</font> = Byte");

    connect(m_ui->ways, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheSim::setWays);
    connect(m_ui->blocks, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheSim::setBlocks);
    connect(m_ui->lines, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheSim::setLines);
    connect(m_ui->sizeBreakdownButton, &QPushButton::clicked, this, &CacheConfigWidget::showSizeBreakdown);

    connect(m_ui->replacementPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        m_cache->setReplacementPolicy(qvariant_cast<CacheSim::ReplPolicy>(m_ui->replacementPolicy->itemData(index)));
    });
    connect(m_ui->wrHit, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        m_cache->setWritePolicy(qvariant_cast<CacheSim::WritePolicy>(m_ui->wrHit->itemData(index)));
    });
    connect(m_ui->wrMiss, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        m_cache->setWriteAllocatePolicy(qvariant_cast<CacheSim::WriteAllocPolicy>(m_ui->wrMiss->itemData(index)));
    });

    connect(m_cache, &CacheSim::configurationChanged, this, &CacheConfigWidget::handleConfigurationChanged);
    connect(m_cache, &CacheSim::configurationChanged, [=] { emit configurationChanged(); });
    connect(m_cache, &CacheSim::hitrateChanged, this, &CacheConfigWidget::updateHitrate);

    setupPresets();
    handleConfigurationChanged();
}

void CacheConfigWidget::showCachePlot() {
    CachePlotWidget plotWidget(*m_cache);
    plotWidget.exec();
}

void CacheConfigWidget::setupPresets() {
    std::vector<std::pair<QString, CacheSim::CachePreset>> presets;

    CacheSim::CachePreset preset;
    preset.wrPolicy = CacheSim::WritePolicy::WriteBack;
    preset.wrAllocPolicy = CacheSim::WriteAllocPolicy::WriteAllocate;
    preset.replPolicy = CacheSim::ReplPolicy::LRU;

    preset.blocks = 2;
    preset.lines = 5;
    preset.ways = 0;
    presets.push_back({"32-entry 4-word direct-mapped", preset});

    preset.blocks = 2;
    preset.lines = 0;
    preset.ways = 5;
    presets.push_back({"32-entry 4-word fully associative", preset});

    preset.blocks = 2;
    preset.lines = 4;
    preset.ways = 1;
    presets.push_back({"32-entry 4-word 2-way set associative", preset});

    for (const auto& preset : presets) {
        m_ui->presets->addItem(preset.first, QVariant::fromValue<CacheSim::CachePreset>(preset.second));
    }

    connect(m_ui->presets, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        const CacheSim::CachePreset preset = qvariant_cast<CacheSim::CachePreset>(m_ui->presets->itemData(index));
        m_justSetPreset = true;
        m_cache->setPreset(preset);
    });
}

void CacheConfigWidget::updateCacheSize() {}

void CacheConfigWidget::handleConfigurationChanged() {
    std::for_each(m_configItems.begin(), m_configItems.end(), [](QObject* o) { o->blockSignals(true); });

    m_ui->ways->setValue(m_cache->getWaysBits());
    m_ui->lines->setValue(m_cache->getLineBits());
    m_ui->blocks->setValue(m_cache->getBlockBits());
    setEnumIndex(m_ui->wrHit, m_cache->getWritePolicy());
    setEnumIndex(m_ui->wrMiss, m_cache->getWriteAllocPolicy());
    setEnumIndex(m_ui->replacementPolicy, m_cache->getReplacementPolicy());

    if (!m_justSetPreset) {
        m_ui->presets->setCurrentIndex(-1);
    }
    m_justSetPreset = false;

    std::for_each(m_configItems.begin(), m_configItems.end(), [](QObject* o) { o->blockSignals(false); });

    updateIndexingText();
    m_ui->size->setText(QString::number(m_cache->getCacheSize().bits));
    updateHitrate();
}

void CacheConfigWidget::updateHitrate() {
    m_ui->hitrate->setText(QString::number(m_cache->getHitRate(), 'G', 4));
    m_ui->hits->setText(QString::number(m_cache->getHits()));
    m_ui->misses->setText(QString::number(m_cache->getMisses()));
    m_ui->writebacks->setText(QString::number(m_cache->getWritebacks()));
}

void CacheConfigWidget::showSizeBreakdown() {
    QString sizeText;

    const auto cacheSize = m_cache->getCacheSize();

    for (const auto& component : cacheSize.components) {
        sizeText += component + "\n";
    }

    sizeText += "\nTotal: " + QString::number(cacheSize.bits) + " Bits";

    QMessageBox::information(this, "Cache Size Breakdown", sizeText);
}

CacheConfigWidget::~CacheConfigWidget() {
    delete m_ui;
}

void CacheConfigWidget::updateIndexingText() {
    QString indexingText = " 0";

    // Byte offset bits
    indexingText = "<font color=\"black\">▊▊</font>" + indexingText;

    // Block offset bits
    QString blocks = "";
    for (int i = 0; i < m_cache->getBlockBits(); i++) {
        blocks += "▊";
    }
    indexingText = "<font color=\"green\">" + blocks + "</font>" + indexingText;

    // Line index bits
    QString index = "";
    for (int i = 0; i < m_cache->getLineBits(); i++) {
        index += "▊";
    }

    indexingText = "<font color=\"red\">" + index + "</font>" + indexingText;

    // Tag index bits
    QString tag = "";
    for (int i = 0; i < m_cache->getTagBits(); i++) {
        tag += "▊";
    }

    indexingText = "<font color=\"gray\">" + tag + "</font>" + indexingText;

    indexingText = "31 " + indexingText;

    m_ui->indexingText->setText(indexingText);
}

}  // namespace Ripes
