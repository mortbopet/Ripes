#include "cachegraphic.h"

#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QPen>

#include "processorhandler.h"
#include "radix.h"

namespace {

// Return a set of all keys in a map
template <typename TK, typename TV>
std::set<TK> keys(std::map<TK, TV> const& input_map) {
    std::set<TK> keyset;
    for (auto const& element : input_map) {
        keyset.insert(element.first);
    }
    return keyset;
}
}  // namespace

namespace Ripes {

CacheGraphic::CacheGraphic(CacheSim& cache) : QGraphicsObject(nullptr), m_cache(cache), m_fm(m_font) {
    connect(&cache, &CacheSim::configurationChanged, this, &CacheGraphic::cacheParametersChanged);
    connect(&cache, &CacheSim::dataChanged, this, &CacheGraphic::dataChanged);
    connect(&cache, &CacheSim::wayInvalidated, this, &CacheGraphic::wayInvalidated);
    connect(&cache, &CacheSim::cacheInvalidated, this, &CacheGraphic::cacheInvalidated);

    cacheParametersChanged();
}

void CacheGraphic::updateLineReplFields(unsigned lineIdx) {
    auto* cacheLine = m_cache.getLine(lineIdx);

    if (cacheLine == nullptr) {
        // Nothing to do
        return;
    }

    if (m_cacheTextItems.at(0).at(0).lru == nullptr) {
        // The current cache configuration does not have any replacement field
        return;
    }

    for (const auto& way : m_cacheTextItems[lineIdx]) {
        // If LRU was just initialized, the actual (software) LRU value may be very large. Mask to the
        // number of actual LRU bits.
        unsigned lruVal = cacheLine->at(way.first).lru;
        lruVal &= generateBitmask(m_cache.getWaysBits());
        const QString lruText = QString::number(lruVal);
        way.second.lru->setText(lruText);

        // LRU text might have changed; update LRU field position to center in column
        const qreal y = lineIdx * m_lineHeight + way.first * m_setHeight;
        const qreal x = m_widthBeforeLRU + m_lruWidth / 2 - m_fm.width(lruText) / 2;

        way.second.lru->setPos(x, y);
    }
}

void CacheGraphic::updateWay(unsigned lineIdx, unsigned wayIdx) {
    CacheWay& way = m_cacheTextItems.at(lineIdx).at(wayIdx);
    const CacheSim::CacheWay& simWay = m_cache.getLine(lineIdx)->at(wayIdx);

    // ======================== Update block text fields ======================
    if (simWay.valid) {
        for (int i = 0; i < m_cache.getBlocks(); i++) {
            QGraphicsSimpleTextItem* blockTextItem = nullptr;
            if (way.blocks.count(i) == 0) {
                // Block text item has not yet been created
                const qreal x =
                    m_widthBeforeBlocks + i * m_blockWidth + (m_blockWidth / 2 - m_fm.width("0x00000000") / 2);
                const qreal y = lineIdx * m_lineHeight + wayIdx * m_setHeight;

                way.blocks[i] = createGraphicsTextItemSP(x, y);
                blockTextItem = way.blocks[i].get();
            } else {
                blockTextItem = way.blocks.at(i).get();
            }

            // Update block text
            const uint32_t addressForBlock = m_cache.buildAddress(simWay.tag, lineIdx, i);
            const auto data = ProcessorHandler::get()->getMemory().readMemConst(addressForBlock);
            const QString text = encodeRadixValue(data, Radix::Hex);
            blockTextItem->setText(text);
            blockTextItem->setToolTip("Address: " + encodeRadixValue(addressForBlock, Radix::Hex));
            // Store the address within the userrole of the block text. Doing this, we are able to easily retrieve the
            // address for the block if the block is clicked.
            blockTextItem->setData(Qt::UserRole, addressForBlock);
        }
    } else {
        // The way is invalid so no block text should be present
        way.blocks.clear();
    }

    // =========================== Update dirty field =========================
    if (way.dirty) {
        way.dirty->setText(QString::number(simWay.dirty));
    }

    // =========================== Update valid field =========================
    if (way.valid) {
        way.valid->setText(QString::number(simWay.valid));
    }

    // ============================ Update tag field ==========================
    if (simWay.valid) {
        QGraphicsSimpleTextItem* tagTextItem = way.tag.get();
        if (tagTextItem == nullptr) {
            const qreal x = m_widthBeforeTag + (m_tagWidth / 2 - m_fm.width("0x00000000") / 2);
            const qreal y = lineIdx * m_lineHeight + wayIdx * m_setHeight;
            way.tag = createGraphicsTextItemSP(x, y);
            tagTextItem = way.tag.get();
        }
        const QString tagText = encodeRadixValue(simWay.tag, Radix::Hex);
        tagTextItem->setText(tagText);
    } else {
        // The way is invalid so no tag text should be present
        if (way.tag) {
            way.tag.reset();
        }
    }

    // ==================== Update dirty blocks highlighting ==================
    const std::set<unsigned> graphicDirtyBlocks = keys(way.dirtyBlocks);
    std::set<unsigned> newDirtyBlocks;
    std::set<unsigned> dirtyBlocksToDelete;
    std::set_difference(graphicDirtyBlocks.begin(), graphicDirtyBlocks.end(), simWay.dirtyBlocks.begin(),
                        simWay.dirtyBlocks.end(), std::inserter(dirtyBlocksToDelete, dirtyBlocksToDelete.begin()));
    std::set_difference(simWay.dirtyBlocks.begin(), simWay.dirtyBlocks.end(), graphicDirtyBlocks.begin(),
                        graphicDirtyBlocks.end(), std::inserter(newDirtyBlocks, newDirtyBlocks.begin()));

    // Delete blocks which are not in sync with the current dirty status of the way
    for (const unsigned& blockToDelete : dirtyBlocksToDelete) {
        way.dirtyBlocks.erase(blockToDelete);
    }
    // Create all required new blocks
    for (const auto& blockIdx : newDirtyBlocks) {
        const auto topLeft =
            QPointF(blockIdx * m_blockWidth + m_widthBeforeBlocks, lineIdx * m_lineHeight + wayIdx * m_setHeight);
        const auto bottomRight = QPointF((blockIdx + 1) * m_blockWidth + m_widthBeforeBlocks,
                                         lineIdx * m_lineHeight + (wayIdx + 1) * m_setHeight);
        way.dirtyBlocks[blockIdx] = std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this);

        const auto& dirtyRectItem = way.dirtyBlocks[blockIdx];
        dirtyRectItem->setZValue(-1);
        dirtyRectItem->setOpacity(0.4);
        dirtyRectItem->setBrush(Qt::darkCyan);
    }
}

QGraphicsSimpleTextItem* CacheGraphic::tryCreateGraphicsTextItem(QGraphicsSimpleTextItem** item, qreal x, qreal y) {
    if (*item == nullptr) {
        *item = new QGraphicsSimpleTextItem(this);
        (*item)->setFont(m_font);
        (*item)->setPos(x, y);
    }
    return *item;
}

std::unique_ptr<QGraphicsSimpleTextItem> CacheGraphic::createGraphicsTextItemSP(qreal x, qreal y) {
    std::unique_ptr<QGraphicsSimpleTextItem> ptr = std::make_unique<QGraphicsSimpleTextItem>(this);
    ptr->setFont(m_font);
    ptr->setPos(x, y);
    return ptr;
}

void CacheGraphic::cacheInvalidated() {
    for (int lineIdx = 0; lineIdx < m_cache.getLines(); lineIdx++) {
        if (const auto* line = m_cache.getLine(lineIdx)) {
            for (const auto& way : *line) {
                updateWay(lineIdx, way.first);
            }
            updateLineReplFields(lineIdx);
        }
    }
}

void CacheGraphic::wayInvalidated(unsigned lineIdx, unsigned wayIdx) {
    updateWay(lineIdx, wayIdx);
    updateLineReplFields(lineIdx);
}

void CacheGraphic::dataChanged(const CacheSim::CacheTransaction* transaction) {
    if (transaction != nullptr) {
        wayInvalidated(transaction->index.line, transaction->index.way);
        updateHighlighting(true, transaction);
    } else {
        updateHighlighting(false, nullptr);
    }
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
        QPointF topLeft = QPointF(0, transaction->index.line * m_lineHeight);
        QPointF bottomRight = QPointF(m_cacheWidth, (transaction->index.line + 1) * m_lineHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* lineRectItem = m_highlightingItems.rbegin()->get();
        lineRectItem->setZValue(-2);
        lineRectItem->setOpacity(0.25);
        lineRectItem->setBrush(Qt::yellow);

        // Draw cache block highlighting rectangle
        topLeft = QPointF(transaction->index.block * m_blockWidth + m_widthBeforeBlocks, 0);
        bottomRight = QPointF((transaction->index.block + 1) * m_blockWidth + m_widthBeforeBlocks, m_cacheHeight);
        m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this));
        auto* blockRectItem = m_highlightingItems.rbegin()->get();
        blockRectItem->setZValue(-2);
        blockRectItem->setOpacity(0.25);
        blockRectItem->setBrush(Qt::yellow);

        // Draw highlighting on the currently accessed block
        topLeft = QPointF(transaction->index.block * m_blockWidth + m_widthBeforeBlocks,
                          transaction->index.line * m_lineHeight + transaction->index.way * m_setHeight);
        bottomRight = QPointF((transaction->index.block + 1) * m_blockWidth + m_widthBeforeBlocks,
                              transaction->index.line * m_lineHeight + (transaction->index.way + 1) * m_setHeight);
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
    for (int lineIdx = 0; lineIdx < m_cache.getLines(); lineIdx++) {
        auto& line = m_cacheTextItems[lineIdx];
        for (int setIdx = 0; setIdx < m_cache.getWays(); setIdx++) {
            const qreal y = lineIdx * m_lineHeight + setIdx * m_setHeight;
            qreal x;

            // Create valid field
            x = m_bitWidth / 2 - m_fm.width("0") / 2;
            line[setIdx].valid = drawText("0", x, y);

            if (m_cache.getWritePolicy() == CacheSim::WritePolicy::WriteBack) {
                // Create dirty bit field
                x = m_widthBeforeDirty + m_bitWidth / 2 - m_fm.width("0") / 2;
                line[setIdx].dirty = drawText("0", x, y);
            }

            if (m_cache.getReplacementPolicy() == CacheSim::ReplPolicy::LRU && m_cache.getWays() > 1) {
                // Create LRU field
                const QString lruText = QString::number(m_cache.getWays() - 1);
                x = m_widthBeforeLRU + m_lruWidth / 2 - m_fm.width(lruText) / 2;
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
    m_setHeight = m_fm.height();
    m_lineHeight = m_setHeight * m_cache.getWays();
    m_blockWidth = m_fm.width(" 0x00000000 ");
    m_bitWidth = m_fm.width("00");
    m_lruWidth = m_fm.width(QString::number(m_cache.getWays()) + " ");
    m_cacheHeight = m_lineHeight * m_cache.getLines();
    m_tagWidth = m_blockWidth;

    // Draw cache:
    new QGraphicsLineItem(0, 0, 0, m_cacheHeight, this);

    qreal width = 0;
    // Draw valid bit column
    new QGraphicsLineItem(m_bitWidth, 0, m_bitWidth, m_cacheHeight, this);
    const QString validBitText = "V";
    auto* validItem = drawText(validBitText, 0, -m_fm.height());
    validItem->setToolTip("Valid bit");
    width += m_bitWidth;

    if (m_cache.getWritePolicy() == CacheSim::WritePolicy::WriteBack) {
        m_widthBeforeDirty = width;

        // Draw dirty bit column
        new QGraphicsLineItem(width + m_bitWidth, 0, width + m_bitWidth, m_cacheHeight, this);
        const QString dirtyBitText = "D";
        auto* dirtyItem = drawText(dirtyBitText, m_widthBeforeDirty, -m_fm.height());
        dirtyItem->setToolTip("Dirty bit");
        width += m_bitWidth;
    }

    m_widthBeforeLRU = width;

    if (m_cache.getReplacementPolicy() == CacheSim::ReplPolicy::LRU && m_cache.getWays() > 1) {
        // Draw LRU bit column
        new QGraphicsLineItem(width + m_lruWidth, 0, width + m_lruWidth, m_cacheHeight, this);
        const QString LRUBitText = "LRU";
        auto* textItem = drawText(LRUBitText, width + m_lruWidth / 2 - m_fm.width(LRUBitText) / 2, -m_fm.height());
        textItem->setToolTip("Least Recently Used bits");
        width += m_lruWidth;
    }

    m_widthBeforeTag = width;

    // Draw tag column
    new QGraphicsLineItem(m_tagWidth + width, 0, m_tagWidth + width, m_cacheHeight, this);
    const QString tagText = "Tag";
    drawText(tagText, width + m_tagWidth / 2 - m_fm.width(tagText) / 2, -m_fm.height());
    width += m_tagWidth;

    m_widthBeforeBlocks = width;

    // Draw horizontal lines between cache blocks
    for (int i = 0; i < m_cache.getBlocks(); i++) {
        const QString blockText = "Block " + QString::number(i);
        drawText(blockText, width + m_tagWidth / 2 - m_fm.width(blockText) / 2, -m_fm.height());
        width += m_blockWidth;
        new QGraphicsLineItem(width, 0, width, m_cacheHeight, this);
    }

    m_cacheWidth = width;

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
        const qreal x = -m_fm.width(text) * 1.2;
        drawText(text, x, y);
    }

    // Draw index column text
    const QString indexText = "Index";
    const qreal x = -m_fm.width(indexText) * 1.2;
    drawText(indexText, x, -m_fm.height());

    initializeControlBits();
}

}  // namespace Ripes
