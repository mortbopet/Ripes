#include "cacheconfigwidget.h"
#include "ui_cacheconfigwidget.h"

#include <QMessageBox>
#include <QSpinBox>
#include <QTimer>

namespace Ripes {

template <typename Enum>
void setupEnumCombobox(QComboBox* combobox, std::map<Enum, QString>& nameMap) {
    for (const auto& iter : nameMap) {
        combobox->addItem(iter.second, QVariant::fromValue(iter.first));
    }
}

template <typename Enum>
void setEnumIndex(QComboBox* combobox, Enum enumItem) {
    for (int i = 0; i < combobox->count(); i++) {
        if (qvariant_cast<Enum>(combobox->itemData(i)) == enumItem) {
            combobox->setCurrentIndex(i);
            return;
        }
    }
    Q_ASSERT(false && "Index not found");
}

CacheConfigWidget::CacheConfigWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheConfigWidget) {
    m_ui->setupUi(this);
}

void CacheConfigWidget::setCache(CacheSim* cache) {
    m_cache = cache;

    const QIcon sizeBreakdownIcon = QIcon(":/icons/info.svg");
    m_ui->sizeBreakdownButton->setIcon(sizeBreakdownIcon);

    setupEnumCombobox(m_ui->replacementPolicy, s_cacheReplPolicyStrings);
    setupEnumCombobox(m_ui->wrHit, s_cacheWritePolicyStrings);
    setupEnumCombobox(m_ui->wrMiss, s_cacheWriteAllocateStrings);

    m_ui->ways->setValue(m_cache->getWaysBits());
    m_ui->lines->setValue(m_cache->getLineBits());
    m_ui->blocks->setValue(m_cache->getBlockBits());

    m_ui->indexingKey->setText(
        "Offsets: <font color=\"gray\">█</font> = Tag <font color=\"red\">█</font> = Index <font "
        "color=\"green\">█</font> = Block <font color=\"black\">█</font> = Byte");

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

    connect(m_cache, &CacheSim::configurationChanged, this, &CacheConfigWidget::configChanged);

    setupPresets();
    m_ui->replacementPolicy->setCurrentIndex(0);

    // For testing purposes only
    connect(m_ui->randomread, &QPushButton::clicked, [=] {
        unsigned address = (std::rand() % 128) & ~0b11;
        m_cache->access(address, CacheSim::AccessType::Read);
    });
    connect(m_ui->randomwrite, &QPushButton::clicked, [=] {
        unsigned address = (std::rand() % 128) & ~0b11;
        m_cache->access(address, CacheSim::AccessType::Write);
    });

    connect(m_ui->undo, &QPushButton::clicked, m_cache, &CacheSim::undo);

    // Synchronize config widgets with initial cache configuration
    configChanged();

    QTimer* timer = new QTimer(this);
    timer->setInterval(20);
    connect(timer, &QTimer::timeout, [=] {
        static int address;
        static int strideCount = 0;

        // Simulate some random access pattern with a bit of spatial locality
        if (strideCount == 0) {
            strideCount = (std::rand() % 32);
            address = (std::rand() % (int)std::pow(2, 14)) & ~0b11;
        } else {
            strideCount--;
            address += 4;
        }

        CacheSim::AccessType type = (std::rand() % 100) > 80 ? CacheSim::AccessType::Write : CacheSim::AccessType::Read;
        m_cache->access(address, type);
    });

    m_ui->autoAccess->setCheckable(true);
    connect(m_ui->autoAccess, &QPushButton::toggled, [=](bool enabled) {
        if (enabled) {
            timer->start();
        } else {
            timer->stop();
        }
    });
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

void CacheConfigWidget::updateCacheSize() {}

void CacheConfigWidget::configChanged() {
    std::vector<QObject*> configItems{m_ui->ways,   m_ui->lines, m_ui->blocks, m_ui->replacementPolicy,
                                      m_ui->wrMiss, m_ui->wrHit};

    std::for_each(configItems.begin(), configItems.end(), [](QObject* o) { o->blockSignals(true); });

    m_ui->ways->setValue(m_cache->getWaysBits());
    m_ui->lines->setValue(m_cache->getLineBits());
    m_ui->blocks->setValue(m_cache->getBlockBits());
    setEnumIndex(m_ui->wrHit, m_cache->getWritePolicy());
    setEnumIndex(m_ui->wrMiss, m_cache->getWriteAllocPolicy());
    setEnumIndex(m_ui->replacementPolicy, m_cache->getReplacementPolicy());

    std::for_each(configItems.begin(), configItems.end(), [](QObject* o) { o->blockSignals(false); });

    updateIndexingText();
    m_ui->size->setText(QString::number(m_cache->getCacheSize().bits));
    setHitRate(m_cache->getHitRate());
}

void CacheConfigWidget::setHitRate(double hitrate) {
    m_ui->hitrate->setText(QString::number(hitrate));
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
    QString indexingText = "0";

    // Byte offset bits
    indexingText = "<font color=\"black\">┃┃</font>" + indexingText;

    // Block offset bits
    QString blocks = "";
    for (int i = 0; i < m_cache->getBlockBits(); i++) {
        blocks += "┃";
    }
    indexingText = "<font color=\"green\">" + blocks + "</font>" + indexingText;

    // Line index bits
    QString index = "";
    for (int i = 0; i < m_cache->getLineBits(); i++) {
        index += "┃";
    }

    indexingText = "<font color=\"red\">" + index + "</font>" + indexingText;

    // Tag index bits
    QString tag = "";
    for (int i = 0; i < m_cache->getTagBits(); i++) {
        tag += "┃";
    }

    indexingText = "<font color=\"gray\">" + tag + "</font>" + indexingText;

    indexingText = "31" + indexingText;

    m_ui->indexingText->setText(indexingText);
}

}  // namespace Ripes
