#pragma once

#include <QFont>
#include <QFontMetrics>
#include <QGraphicsItem>
#include <QObject>
#include <memory>
#include "cachesim.h"

namespace Ripes {

class CacheGraphic : public QGraphicsObject {
public:
    CacheGraphic(CacheSim& cache);

    QRectF boundingRect() const override;

    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* = nullptr) override {}

public slots:
    /**
     * @brief dataChanged
     * The cache simulator indicates that some entries in the cache has changed. CacheGraphic will, using @p
     * transaction, lazily initialize and update all required values to reflect the new state of the cache.
     */
    void dataChanged(const CacheSim::CacheTransaction* transaction);

    /**
     * @brief wayInvalidated
     * The cache simulator has signalled that all graphics in the given cache way shall be reinitialized to reflect a
     * changed state in the cache simulator.
     */
    void wayInvalidated(unsigned lineIdx, unsigned wayIdx);

    /**
     * @brief cacheInvalidated
     * The cache simulator has signalled that the entirety of the cache simulator graphical view should be reloaded.
     */
    void cacheInvalidated();

    /**
     * @brief cacheParametersChanged
     * Recalculates and redraws the graphic based on the current cache parameters
     */
    void cacheParametersChanged();

    void reset();

private:
    // Data structure modelling the cache; keeping graphics text items for each entry
    // All text items which are not always present are stored as unqiue_ptr's to facilitate easy deletion when undoing
    // changes to the cache
    struct CacheWay {
        std::map<unsigned, std::unique_ptr<QGraphicsSimpleTextItem>> blocks;
        std::unique_ptr<QGraphicsSimpleTextItem> tag = nullptr;
        QGraphicsSimpleTextItem* lru = nullptr;
        QGraphicsSimpleTextItem* valid = nullptr;
        QGraphicsSimpleTextItem* dirty = nullptr;
        std::map<unsigned, std::unique_ptr<QGraphicsRectItem>> dirtyBlocks;
    };

    using CacheLine = std::map<unsigned, CacheWay>;

    /**
     * @brief initializeControlBits
     * Constructs all of the "Valid" and "LRU" text items within the cache
     */
    void initializeControlBits();
    void updateHighlighting(bool active, const CacheSim::CacheTransaction* transaction);
    QGraphicsSimpleTextItem* drawText(const QString& text, qreal x, qreal y);
    QGraphicsSimpleTextItem* tryCreateGraphicsTextItem(QGraphicsSimpleTextItem** item, qreal x, qreal y);
    std::unique_ptr<QGraphicsSimpleTextItem> createGraphicsTextItemSP(qreal x, qreal y);

    // Graphical update functions
    void updateLineReplFields(unsigned lineIdx);
    void updateWay(unsigned lineIdx, unsigned wayIdx);

    QFont m_font = QFont("Inconsolata", 12);
    CacheSim& m_cache;

    std::vector<std::unique_ptr<QGraphicsRectItem>> m_highlightingItems;

    QFontMetricsF m_fm;

    // Drawing dimensions
    qreal m_setHeight = 0;
    qreal m_lineHeight = 0;
    qreal m_blockWidth = 0;
    qreal m_bitWidth = 0;
    qreal m_cacheHeight = 0;
    qreal m_tagWidth = 0;
    qreal m_cacheWidth = 0;
    qreal m_widthBeforeBlocks = 0;
    qreal m_widthBeforeTag = 0;
    qreal m_widthBeforeLRU = 0;
    qreal m_widthBeforeDirty = 0;
    qreal m_lruWidth = 0;

    /**
     * @brief m_cacheTextItems
     * All text items in the cache are managed in @var m_cacheTextItems.
     * This object models the hierarchy of the cache, and stores all currently initialized text items for the cache.
     * The object is indexed similarly to how the cache simulator is indexed. As such, a cache transaction is used to
     * traverse the object.
     * All items are lazily initialized in calls to dataChanged(). This prevents initializing a ton of items if a user
     * has created a very large cache.
     */
    std::map<unsigned, CacheLine> m_cacheTextItems;
};

}  // namespace Ripes
