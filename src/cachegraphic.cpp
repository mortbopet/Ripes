#include "cachegraphic.h"

#include <QFontMetrics>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QPen>

namespace Ripes {

CacheGraphic::CacheGraphic(CacheBase& cache) : QGraphicsObject(nullptr), m_cache(cache) {
    connect(&cache, &CacheBase::parametersChanged, this, &CacheGraphic::cacheParametersChanged);
    cacheParametersChanged();
}

void CacheGraphic::drawText(const QString& text, qreal x, qreal y) {
    auto* textItem = new QGraphicsSimpleTextItem(text, this);
    textItem->setFont(m_font);
    textItem->setPos(x, y);
}

void CacheGraphic::cacheParametersChanged() {
    prepareGeometryChange();

    // Remove all items
    for (const auto& item : childItems())
        delete item;

    // Determine cell dimensions
    auto metrics = QFontMetricsF(m_font);
    const qreal setHeight = metrics.height();
    const qreal lineHeight = setHeight * m_cache.getSets();
    const qreal blockWidth = metrics.horizontalAdvance("0xFFFFFFFF");
    const qreal bitWidth = metrics.horizontalAdvance("0");
    const qreal cacheHeight = lineHeight * m_cache.getLines();
    const qreal tagWidth = blockWidth;

    // Draw cache:

    qreal horizontalAdvance = 0;
    // Draw valid bit column
    new QGraphicsLineItem(0, 0, 0, cacheHeight, this);
    new QGraphicsLineItem(bitWidth, 0, bitWidth, cacheHeight, this);
    const QString validBitText = "V";
    drawText(validBitText, 0, -metrics.height());
    horizontalAdvance += bitWidth;

    // Draw tag column
    new QGraphicsLineItem(tagWidth, 0, tagWidth, cacheHeight, this);
    const QString tagText = "Tag";
    drawText(tagText, horizontalAdvance + tagWidth / 2 - metrics.horizontalAdvance(tagText) / 2, -metrics.height());
    horizontalAdvance += tagWidth;

    // Draw horizontal lines between cache blocks
    for (int i = 0; i < m_cache.getBlocks(); i++) {
        const QString blockText = "Block " + QString::number(i);
        drawText(blockText, horizontalAdvance + tagWidth / 2 - metrics.horizontalAdvance(blockText) / 2,
                 -metrics.height());
        horizontalAdvance += blockWidth;
        auto* l3 = new QGraphicsLineItem(horizontalAdvance, 0, horizontalAdvance, cacheHeight, this);
    }

    // Draw cache line rows
    for (int i = 0; i <= m_cache.getLines(); i++) {
        qreal verticalAdvance = i * lineHeight;
        new QGraphicsLineItem(0, verticalAdvance, horizontalAdvance, verticalAdvance, this);

        if (i < m_cache.getLines()) {
            // Draw cache set rows
            for (int j = 1; j < m_cache.getSets(); j++) {
                verticalAdvance += setHeight;
                auto* setLine = new QGraphicsLineItem(0, verticalAdvance, horizontalAdvance, verticalAdvance, this);
                auto pen = setLine->pen();
                pen.setStyle(Qt::DashLine);
                setLine->setPen(pen);
            }
        }
    }

    // Draw line index numbers
    for (int i = 0; i < m_cache.getLines(); i++) {
        const QString text = QString::number(i);

        const qreal y = i * lineHeight + lineHeight / 2 - setHeight / 2;
        const qreal x = -metrics.horizontalAdvance(text) * 1.2;
        drawText(text, x, y);
    }
}

}  // namespace Ripes
