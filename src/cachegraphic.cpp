#include "cachegraphic.h"

#include <QFontMetrics>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QPen>

#include "processorhandler.h"
#include "radix.h"

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
    const unsigned setIdx = m_cache.getAccessWayIdx();

    const auto data = ProcessorHandler::get()->getMemory().readMemConst(address);
    auto fm = QFontMetricsF(m_font);

    auto& cacheline = m_cacheTextItems[lineIdx][setIdx];

    // Try to locate a previously created text object for the block.
    QGraphicsSimpleTextItem* blockTextItem = cacheline.blocks[blockIdx];
    if (blockTextItem == nullptr) {
        blockTextItem = new QGraphicsSimpleTextItem(this);
        blockTextItem->setFont(m_font);
        cacheline.blocks[blockIdx] = blockTextItem;
        const qreal x =
            m_widthBeforeBlocks + blockIdx * m_blockWidth + (m_blockWidth / 2 - fm.horizontalAdvance("0x00000000") / 2);
        const qreal y = lineIdx * m_lineHeight + setIdx * m_setHeight;
        blockTextItem->setPos(x, y);
    }
    const QString text = encodeRadixValue(data, Radix::Hex);
    blockTextItem->setText(text);

    // Update tag
    QGraphicsSimpleTextItem* tagTextItem = cacheline.tag;
    if (tagTextItem == nullptr) {
        // Create tag
        tagTextItem = new QGraphicsSimpleTextItem(this);
        tagTextItem->setFont(m_font);
        cacheline.tag = tagTextItem;
        auto fm = QFontMetricsF(m_font);
        const qreal x = m_widthBeforeTag + (m_tagWidth / 2 - fm.horizontalAdvance("0x00000000") / 2);
        const qreal y = lineIdx * m_lineHeight + setIdx * m_setHeight;
        tagTextItem->setPos(x, y);
    }
    const QString tagText = encodeRadixValue(m_cache.getAccessTag(), Radix::Hex);
    tagTextItem->setText(tagText);

    // Update valid & LRU text
    // A value cannot be invalidated with the current schema (uniprocessor)
    cacheline.valid->setText(QString::number(1));
    if (m_cache.getReplacementPolicy() == CacheReplPlcy::LRU && m_cache.getWays() > 1) {
        if (m_cache.getCurrentAccessLine()) {
            auto* currentAccessLine = m_cache.getCurrentAccessLine();
            for (const auto& way : m_cacheTextItems[m_cache.getAccessLineIdx()]) {
                // If LRU was just initialized, the actual (software) LRU value may be very large. Mask to the number of
                // actual LRU bits.
                unsigned lruVal = currentAccessLine->at(way.first).lru;
                lruVal &= generateBitmask(m_cache.getWaysBits());
                const QString lruText = QString::number(lruVal);
                way.second.lru->setText(lruText);

                // LRU text might have changed; update LRU field position to center in column
                const qreal y = lineIdx * m_lineHeight + way.first * m_setHeight;
                const qreal x = m_bitWidth + m_lruWidth / 2 - fm.horizontalAdvance(lruText) / 2;

                way.second.lru->setPos(x, y);
            }
        }
    }

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
        const unsigned setIdx = m_cache.getAccessWayIdx();

        // Draw cache line highlighting rectangle
        QPointF topLeft = QPointF(0, lineIdx * m_lineHeight);
        QPointF bottomRight = QPointF(m_cacheWidth, (lineIdx + 1) * m_lineHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* lineRectItem = m_highlightingItems.rbegin()->get();
        lineRectItem->setZValue(-2);
        lineRectItem->setOpacity(0.25);
        lineRectItem->setBrush(Qt::yellow);

        // Draw cache block highlighting rectangle
        topLeft = QPointF(blockIdx * m_blockWidth + m_widthBeforeBlocks, 0);
        bottomRight = QPointF((blockIdx + 1) * m_blockWidth + m_widthBeforeBlocks, m_cacheHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* blockRectItem = m_highlightingItems.rbegin()->get();
        blockRectItem->setZValue(-2);
        blockRectItem->setOpacity(0.25);
        blockRectItem->setBrush(Qt::yellow);

        // Draw highlighting on the currently accessed block
        topLeft = QPointF(blockIdx * m_blockWidth + m_widthBeforeBlocks, lineIdx * m_lineHeight + setIdx * m_setHeight);
        bottomRight = QPointF((blockIdx + 1) * m_blockWidth + m_widthBeforeBlocks,
                              lineIdx * m_lineHeight + (setIdx + 1) * m_setHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* hitRectItem = m_highlightingItems.rbegin()->get();
        hitRectItem->setZValue(-1);
        hitRectItem->setOpacity(0.4);
        if (m_cache.isCacheHit()) {
            hitRectItem->setBrush(Qt::green);
        } else {
            hitRectItem->setBrush(Qt::red);
        }
    }
}

void CacheGraphic::initializeControlBits() {
    auto fm = QFontMetricsF(m_font);

    for (int lineIdx = 0; lineIdx < m_cache.getLines(); lineIdx++) {
        auto& line = m_cacheTextItems[lineIdx];
        for (int setIdx = 0; setIdx < m_cache.getWays(); setIdx++) {
            const qreal y = lineIdx * m_lineHeight + setIdx * m_setHeight;
            qreal x;

            // Create valid field
            auto* validItem = new QGraphicsSimpleTextItem(this);
            validItem->setFont(m_font);
            validItem->setText("0");
            x = fm.horizontalAdvance(validItem->text()) / 2;
            validItem->setPos(x, y);
            line[setIdx].valid = validItem;

            if (m_cache.getReplacementPolicy() == CacheReplPlcy::LRU && m_cache.getWays() > 1) {
                // Create LRU field
                auto* lruItem = new QGraphicsSimpleTextItem(this);
                lruItem->setFont(m_font);
                const QString lruText = QString::number(m_cache.getWays() - 1);
                lruItem->setText(lruText);
                x = m_bitWidth + m_lruWidth / 2 - fm.horizontalAdvance(lruText) / 2;
                lruItem->setPos(x, y);
                line[setIdx].lru = lruItem;
            }
        }
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
    m_lineHeight = m_setHeight * m_cache.getWays();
    m_blockWidth = metrics.horizontalAdvance(" 0x00000000 ");
    m_bitWidth = metrics.horizontalAdvance("00");
    m_lruWidth = metrics.horizontalAdvance(QString::number(m_cache.getWays()) + " ");
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

    if (m_cache.getReplacementPolicy() == CacheReplPlcy::LRU && m_cache.getWays() > 1) {
        // Draw LRU bit column
        new QGraphicsLineItem(horizontalAdvance, 0, horizontalAdvance, m_cacheHeight, this);
        new QGraphicsLineItem(m_bitWidth + m_lruWidth, 0, m_bitWidth + m_lruWidth, m_cacheHeight, this);
        const QString LRUBitText = "LRU";
        drawText(LRUBitText, horizontalAdvance + m_lruWidth / 2 - metrics.horizontalAdvance(LRUBitText) / 2,
                 -metrics.height());
        horizontalAdvance += m_lruWidth;
    }

    m_widthBeforeTag = horizontalAdvance;

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
            for (int j = 1; j < m_cache.getWays(); j++) {
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

    // Draw index column text
    const QString indexText = "Index";
    const qreal x = -metrics.horizontalAdvance(indexText) * 1.2;
    drawText(indexText, x, -metrics.height());

    initializeControlBits();
}

}  // namespace Ripes
