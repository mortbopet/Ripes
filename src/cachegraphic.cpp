#include "cachegraphic.h"

#include <QFontMetrics>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QPen>

#include "processorhandler.h"

namespace Ripes {

CacheGraphic::CacheGraphic(CacheBase& cache) : QGraphicsObject(nullptr), m_cache(cache) {
    connect(&cache, &CacheBase::configurationChanged, this, &CacheGraphic::cacheParametersChanged);
    connect(&cache, &CacheBase::accessChanged, this, &CacheGraphic::updateHighlighting);
    connect(&cache, &CacheBase::dataChanged, this, &CacheGraphic::dataChanged);

    cacheParametersChanged();
}

void CacheGraphic::dataChanged(uint32_t address) {
    const unsigned lineIdx = m_cache.getAccessLineIdx();
    const unsigned blockIdx = m_cache.getAccessBlockIdx();
    const unsigned setIdx = m_cache.getAccessSetIdx();

    const auto data = ProcessorHandler::get()->getMemory().readMemConst(address);

    // Try to locate a previously created text object for the block.
    QGraphicsSimpleTextItem* blockTextItem = m_cacheTextItems[lineIdx][setIdx][blockIdx];
    if (blockTextItem == nullptr) {
        blockTextItem = new QGraphicsSimpleTextItem(this);
        m_cacheTextItems[lineIdx][setIdx][blockIdx] = blockTextItem;
    }

    blockTextItem->setText("0x" + QString::number(data, 16));

    const qreal x = m_widthBeforeBlocks + blockIdx * m_blockWidth;
    const qreal y = lineIdx * m_lineHeight + setIdx * m_setHeight;
    blockTextItem->setPos(x, y);
    updateHighlighting(true);
}

void CacheGraphic::drawText(const QString& text, qreal x, qreal y) {
    auto* textItem = new QGraphicsSimpleTextItem(text, this);
    textItem->setFont(m_font);
    textItem->setPos(x, y);
}

void CacheGraphic::updateHighlighting(bool active) {
    m_highlightingItems.clear();

    if (active) {
        // Redraw highlighting rectangles indicating the current indexing
        const unsigned lineIdx = m_cache.getAccessLineIdx();
        const unsigned blockIdx = m_cache.getAccessBlockIdx();

        // Draw cache line highlighting rectangle
        QPointF topLeft = QPointF(0, lineIdx * m_lineHeight);
        QPointF bottomRight = QPointF(m_cacheWidth, (lineIdx + 1) * m_lineHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* lineRectItem = m_highlightingItems.rbegin()->get();
        lineRectItem->setZValue(-1);
        lineRectItem->setOpacity(0.25);
        lineRectItem->setBrush(Qt::red);

        // Draw cache block highlighting rectangle
        topLeft = QPointF(blockIdx * m_blockWidth + m_widthBeforeBlocks, 0);
        bottomRight = QPointF((blockIdx + 1) * m_blockWidth + m_widthBeforeBlocks, m_cacheHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* blockRectItem = m_highlightingItems.rbegin()->get();
        blockRectItem->setZValue(-1);
        blockRectItem->setOpacity(0.25);
        blockRectItem->setBrush(Qt::yellow);
    }
}

void CacheGraphic::cacheParametersChanged() {
    prepareGeometryChange();

    // Remove all items
    m_highlightingItems.clear();
    m_cacheTextItems.clear();
    for (const auto& item : childItems())
        delete item;

    // Determine cell dimensions
    auto metrics = QFontMetricsF(m_font);
    m_setHeight = metrics.height();
    m_lineHeight = m_setHeight * m_cache.getSets();
    m_blockWidth = metrics.horizontalAdvance("0xFFFFFFFF");
    m_bitWidth = metrics.horizontalAdvance("00");
    m_cacheHeight = m_lineHeight * m_cache.getLines();
    m_tagWidth = m_blockWidth;

    // Draw cache:

    qreal horizontalAdvance = 0;
    // Draw valid bit column
    new QGraphicsLineItem(0, 0, 0, m_cacheHeight, this);
    new QGraphicsLineItem(m_bitWidth, 0, m_bitWidth, m_cacheHeight, this);
    const QString validBitText = "V";
    drawText(validBitText, 0, -metrics.height());
    horizontalAdvance += m_bitWidth;

    // Draw tag column
    new QGraphicsLineItem(m_tagWidth + horizontalAdvance, 0, m_tagWidth + horizontalAdvance, m_cacheHeight, this);
    const QString tagText = "Tag";
    drawText(tagText, horizontalAdvance + m_tagWidth / 2 - metrics.horizontalAdvance(tagText) / 2, -metrics.height());
    horizontalAdvance += m_tagWidth;

    m_widthBeforeBlocks = horizontalAdvance;

    // Draw horizontal lines between cache blocks
    for (int i = 0; i < m_cache.getBlocks(); i++) {
        const QString blockText = "Block " + QString::number(i);
        drawText(blockText, horizontalAdvance + m_tagWidth / 2 - metrics.horizontalAdvance(blockText) / 2,
                 -metrics.height());
        horizontalAdvance += m_blockWidth;
        auto* l3 = new QGraphicsLineItem(horizontalAdvance, 0, horizontalAdvance, m_cacheHeight, this);
    }

    m_cacheWidth = horizontalAdvance;

    // Draw cache line rows
    for (int i = 0; i <= m_cache.getLines(); i++) {
        qreal verticalAdvance = i * m_lineHeight;
        new QGraphicsLineItem(0, verticalAdvance, m_cacheWidth, verticalAdvance, this);

        if (i < m_cache.getLines()) {
            // Draw cache set rows
            for (int j = 1; j < m_cache.getSets(); j++) {
                verticalAdvance += m_setHeight;
                auto* setLine = new QGraphicsLineItem(0, verticalAdvance, m_cacheWidth, verticalAdvance, this);
                auto pen = setLine->pen();
                pen.setStyle(Qt::DashLine);
                setLine->setPen(pen);
            }
        }
    }

    // Draw line index numbers
    for (int i = 0; i < m_cache.getLines(); i++) {
        const QString text = QString::number(i);

        const qreal y = i * m_lineHeight + m_lineHeight / 2 - m_setHeight / 2;
        const qreal x = -metrics.horizontalAdvance(text) * 1.2;
        drawText(text, x, y);
    }
}

}  // namespace Ripes
