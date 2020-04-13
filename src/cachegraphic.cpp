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

CacheGraphic::CacheGraphic(CacheSim& cache) : QGraphicsObject(nullptr), m_cache(cache) {
    connect(&cache, &CacheSim::configurationChanged, this, &CacheGraphic::cacheParametersChanged);
    connect(&cache, &CacheSim::dataChanged, this, &CacheGraphic::dataChanged);

    cacheParametersChanged();
}

QGraphicsSimpleTextItem* CacheGraphic::tryCreateGraphicsTextItem(QGraphicsSimpleTextItem** item, qreal x, qreal y) {
    if (*item == nullptr) {
        *item = new QGraphicsSimpleTextItem(this);
        (*item)->setFont(m_font);
        (*item)->setPos(x, y);
    }
    return *item;
}

void CacheGraphic::dataChanged(const CacheSim::CacheTransaction& transaction) {
    auto fm = QFontMetricsF(m_font);

    if (!transaction.isHit && transaction.type == CacheSim::AccessType::Write &&
        m_cache.getWriteAllocPolicy() == CacheSim::WriteAllocPolicy::NoWriteAllocate) {
        // Nothing to do graphically; the access was a miss and we are not write allocating. So there is no new line
        // written into the cache => no line to be set dirty.
        return;
    }

    auto& way = m_cacheTextItems[transaction.lineIdx][transaction.wayIdx];
    auto& simWay = m_cache.getLine(transaction.lineIdx)->at(transaction.wayIdx);

    if (transaction.isHit) {
        /* The access is a hit. This implies that:
         * - There exists a previous cycle where text objects were initialized for the given cache line
         * - Given the above, we only need to update the block which the access is indexing to
         */

        const qreal x = m_widthBeforeBlocks + transaction.blockIdx * m_blockWidth +
                        (m_blockWidth / 2 - fm.horizontalAdvance("0x00000000") / 2);
        const qreal y = transaction.lineIdx * m_lineHeight + transaction.wayIdx * m_setHeight;

        const auto data = ProcessorHandler::get()->getMemory().readMemConst(transaction.address);
        QGraphicsSimpleTextItem* blockTextItem = way.blocks[transaction.blockIdx];
        const QString text = encodeRadixValue(data, Radix::Hex);
        blockTextItem->setText(text);
        blockTextItem->setToolTip("Address: " + encodeRadixValue(transaction.address, Radix::Hex));
    } else {
        /* The access is a miss. This implies that:
         * - The entirety of the cache way must be reread/reinitialized given that a new cache line is to be read.
         */

        // Update all blocks
        for (unsigned i = 0; i < m_cache.getBlocks(); i++) {
            const qreal x =
                m_widthBeforeBlocks + i * m_blockWidth + (m_blockWidth / 2 - fm.horizontalAdvance("0x00000000") / 2);
            const qreal y = transaction.lineIdx * m_lineHeight + transaction.wayIdx * m_setHeight;

            QGraphicsSimpleTextItem* blockTextItem = tryCreateGraphicsTextItem(&way.blocks[i], x, y);
            const uint32_t addressForBlock = (transaction.address & ~m_cache.getBlockMask()) | (i << 2);
            const auto data = ProcessorHandler::get()->getMemory().readMemConst(addressForBlock);
            const QString text = encodeRadixValue(data, Radix::Hex);
            blockTextItem->setText(text);
            blockTextItem->setToolTip("Address: " + encodeRadixValue(addressForBlock, Radix::Hex));
        }

        // Update valid
        // Given that a value cannot be invalidated with the current schema (uniprocessor), the valid bit is simply
        // flipped if we see a transition
        if (transaction.transToValid) {
            way.valid->setText(QString::number(1));
        }
    }

    // Update dirty field
    if (transaction.isHit && m_cache.getWritePolicy() == CacheSim::WritePolicy::WriteBack) {
        way.dirty->setText(QString::number(simWay.dirty));
    }

    // Update tag
    if (transaction.tagChanged) {
        // Tag changing implies that the entries of the way was evicted. Clear any highlighting blocks for the way,
        // before setting new write blocks.
        way.dirtyBlocks.clear();

        const qreal x = m_widthBeforeTag + (m_tagWidth / 2 - fm.horizontalAdvance("0x00000000") / 2);
        const qreal y = transaction.lineIdx * m_lineHeight + transaction.wayIdx * m_setHeight;

        QGraphicsSimpleTextItem* tagTextItem = tryCreateGraphicsTextItem(&way.tag, x, y);
        const QString tagText = encodeRadixValue(transaction.tag, Radix::Hex);
        tagTextItem->setText(tagText);
    }

    // Update dirty blocks
    if (transaction.type == CacheSim::AccessType::Write &&
        m_cache.getWritePolicy() == CacheSim::WritePolicy::WriteBack) {
        if (way.dirtyBlocks.count(transaction.blockIdx) == 0) {
            // The block was dirtied by this transaction; create a dirty rect
            const auto topLeft = QPointF(transaction.blockIdx * m_blockWidth + m_widthBeforeBlocks,
                                         transaction.lineIdx * m_lineHeight + transaction.wayIdx * m_setHeight);
            const auto bottomRight =
                QPointF((transaction.blockIdx + 1) * m_blockWidth + m_widthBeforeBlocks,
                        transaction.lineIdx * m_lineHeight + (transaction.wayIdx + 1) * m_setHeight);
            way.dirtyBlocks[transaction.blockIdx] =
                std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this);

            const auto& dirtyRectItem = way.dirtyBlocks[transaction.blockIdx];
            dirtyRectItem->setZValue(-1);
            dirtyRectItem->setOpacity(0.4);
            dirtyRectItem->setBrush(Qt::darkCyan);
        }
    }

    // Update LRU fields.
    // This field must be updated for all ways in a set
    bool updateLRU = true;
    updateLRU &= m_cache.getReplacementPolicy() == CacheSim::ReplPolicy::LRU && m_cache.getWays() > 1;
    // Special case handling for writing to a missed block with no write alloc. In this case, we do not update LRU bits
    // because technically the write does not map to any cache line.
    updateLRU &= !(!transaction.isHit && m_cache.getWriteAllocPolicy() == CacheSim::WriteAllocPolicy::NoWriteAllocate);

    if (updateLRU) {
        if (auto* currentAccessLine = m_cache.getLine(transaction.lineIdx)) {
            for (const auto& way : m_cacheTextItems[transaction.lineIdx]) {
                // If LRU was just initialized, the actual (software) LRU value may be very large. Mask to the
                // number of actual LRU bits.
                unsigned lruVal = currentAccessLine->at(way.first).lru;
                lruVal &= generateBitmask(m_cache.getWaysBits());
                const QString lruText = QString::number(lruVal);
                way.second.lru->setText(lruText);

                // LRU text might have changed; update LRU field position to center in column
                const qreal y = transaction.lineIdx * m_lineHeight + way.first * m_setHeight;
                const qreal x = m_widthBeforeLRU + m_lruWidth / 2 - fm.horizontalAdvance(lruText) / 2;

                way.second.lru->setPos(x, y);
            }
        }
    }

    updateHighlighting(true, &transaction);
}

QGraphicsSimpleTextItem* CacheGraphic::drawText(const QString& text, qreal x, qreal y) {
    auto* textItem = new QGraphicsSimpleTextItem(text, this);
    textItem->setFont(m_font);
    textItem->setPos(x, y);
    return textItem;
}

void CacheGraphic::updateHighlighting(bool active, const CacheSim::CacheTransaction* transaction) {
    m_highlightingItems.clear();

    if (active) {
        // Redraw highlighting rectangles indicating the current indexing

        // Draw cache line highlighting rectangle
        QPointF topLeft = QPointF(0, transaction->lineIdx * m_lineHeight);
        QPointF bottomRight = QPointF(m_cacheWidth, (transaction->lineIdx + 1) * m_lineHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* lineRectItem = m_highlightingItems.rbegin()->get();
        lineRectItem->setZValue(-2);
        lineRectItem->setOpacity(0.25);
        lineRectItem->setBrush(Qt::yellow);

        // Draw cache block highlighting rectangle
        topLeft = QPointF(transaction->blockIdx * m_blockWidth + m_widthBeforeBlocks, 0);
        bottomRight = QPointF((transaction->blockIdx + 1) * m_blockWidth + m_widthBeforeBlocks, m_cacheHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* blockRectItem = m_highlightingItems.rbegin()->get();
        blockRectItem->setZValue(-2);
        blockRectItem->setOpacity(0.25);
        blockRectItem->setBrush(Qt::yellow);

        // Draw highlighting on the currently accessed block
        topLeft = QPointF(transaction->blockIdx * m_blockWidth + m_widthBeforeBlocks,
                          transaction->lineIdx * m_lineHeight + transaction->wayIdx * m_setHeight);
        bottomRight = QPointF((transaction->blockIdx + 1) * m_blockWidth + m_widthBeforeBlocks,
                              transaction->lineIdx * m_lineHeight + (transaction->wayIdx + 1) * m_setHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* hitRectItem = m_highlightingItems.rbegin()->get();
        hitRectItem->setZValue(-1);
        if (transaction->isHit) {
            hitRectItem->setOpacity(0.4);
            hitRectItem->setBrush(Qt::green);
        } else {
            hitRectItem->setOpacity(0.8);
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
            x = m_bitWidth / 2 - fm.horizontalAdvance("0") / 2;
            line[setIdx].valid = drawText("0", x, y);

            if (m_cache.getWritePolicy() == CacheSim::WritePolicy::WriteBack) {
                // Create dirty bit field
                x = m_widthBeforeDirty + m_bitWidth / 2 - fm.horizontalAdvance("0") / 2;
                line[setIdx].dirty = drawText("0", x, y);
            }

            if (m_cache.getReplacementPolicy() == CacheSim::ReplPolicy::LRU && m_cache.getWays() > 1) {
                // Create LRU field
                const QString lruText = QString::number(m_cache.getWays() - 1);
                x = m_widthBeforeLRU + m_lruWidth / 2 - fm.horizontalAdvance(lruText) / 2;
                line[setIdx].lru = drawText(lruText, x, y);
            }
        }
    }
}

QRectF CacheGraphic::boundingRect() const {
    // We do not paint anything in Cachegraphic; only instantiate other QGraphicsItem-derived objects. So just return
    // the bounding rect of child items
    return childrenBoundingRect();
}

void CacheGraphic::cacheParametersChanged() {
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
    new QGraphicsLineItem(0, 0, 0, m_cacheHeight, this);

    qreal horizontalAdvance = 0;
    // Draw valid bit column
    new QGraphicsLineItem(m_bitWidth, 0, m_bitWidth, m_cacheHeight, this);
    const QString validBitText = "V";
    auto* validItem = drawText(validBitText, 0, -metrics.height());
    validItem->setToolTip("Valid bit");
    horizontalAdvance += m_bitWidth;

    if (m_cache.getWritePolicy() == CacheSim::WritePolicy::WriteBack) {
        m_widthBeforeDirty = horizontalAdvance;

        // Draw dirty bit column
        new QGraphicsLineItem(horizontalAdvance + m_bitWidth, 0, horizontalAdvance + m_bitWidth, m_cacheHeight, this);
        const QString dirtyBitText = "D";
        auto* dirtyItem = drawText(dirtyBitText, m_widthBeforeDirty, -metrics.height());
        dirtyItem->setToolTip("Dirty bit");
        horizontalAdvance += m_bitWidth;
    }

    m_widthBeforeLRU = horizontalAdvance;

    if (m_cache.getReplacementPolicy() == CacheSim::ReplPolicy::LRU && m_cache.getWays() > 1) {
        // Draw LRU bit column
        new QGraphicsLineItem(horizontalAdvance + m_lruWidth, 0, horizontalAdvance + m_lruWidth, m_cacheHeight, this);
        const QString LRUBitText = "LRU";
        auto* textItem =
            drawText(LRUBitText, horizontalAdvance + m_lruWidth / 2 - metrics.horizontalAdvance(LRUBitText) / 2,
                     -metrics.height());
        textItem->setToolTip("Least Recently Used bits");
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
