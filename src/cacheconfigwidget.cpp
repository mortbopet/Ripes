#include "cacheconfigwidget.h"
#include "ui_cacheconfigwidget.h"

#include <QMessageBox>
#include <QSpinBox>

namespace Ripes {

CacheConfigWidget::CacheConfigWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheConfigWidget) {
    m_ui->setupUi(this);
}

void CacheConfigWidget::setCache(CacheBase* cache) {
    m_cache = cache;

    connect(m_ui->ways, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheBase::setWays);
    connect(m_ui->blocks, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheBase::setBlocks);
    connect(m_ui->lines, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheBase::setLines);

    connect(m_ui->sizeBreakdownButton, &QPushButton::clicked, this, &CacheConfigWidget::showSizeBreakdown);

    m_ui->ways->setValue(m_cache->getWaysBits());
    m_ui->lines->setValue(m_cache->getLineBits());
    m_ui->blocks->setValue(m_cache->getBlockBits());
}

void CacheConfigWidget::setCacheSize(unsigned size) {
    m_ui->size->setText(QString::number(size));
}

void CacheConfigWidget::setHitRate(double hitrate) {
    m_ui->hitrate->setText(QString::number(hitrate));
}

void CacheConfigWidget::showSizeBreakdown() {
    QString sizeText;
    sizeText += "Cache size components:\n\n";

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
