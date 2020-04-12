#pragma once

#include <QFont>
#include <QGraphicsItem>
#include <QObject>
#include <memory>
#include "cachesim.h"

namespace Ripes {

class CacheGraphic : public QGraphicsObject {
public:
    CacheGraphic(CacheSim& cache);

    QRectF boundingRect() const override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override {}

public slots:
    /**
     * @brief dataChanged
     * The cache simulator indicates that some entries in the cache has changed. CacheGraphic will, using @p
     * transaction, lazily initialize and update all required values to reflect the new state of the cache.
     */
    void dataChanged(const CacheSim::CacheTransaction& transaction);

    /**
     * @brief cacheParametersChanged
     * Recalculates and redraws the graphic based on the current cache parameters
     */
    void cacheParametersChanged();

    void reset();

private:
    /**
     * @brief initializeControlBits
     * Constructs all of the "Valid" and "LRU" text items within the cache
     */
    void initializeControlBits();
    void updateHighlighting(bool active, const CacheSim::CacheTransaction* transaction);
    QGraphicsSimpleTextItem* drawText(const QString& text, qreal x, qreal y);
    QGraphicsSimpleTextItem* tryCreateGraphicsTextItem(QGraphicsSimpleTextItem** item, qreal x, qreal y);

    QFont m_font = QFont("Inconsolata", 12);
    CacheSim& m_cache;

    std::vector<std::unique_ptr<QGraphicsRectItem>> m_highlightingItems;

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

    // Data structure modelling the cache; keeping graphics text items for each entry
    struct CacheWay {
        std::map<unsigned, QGraphicsSimpleTextItem*> blocks;
        QGraphicsSimpleTextItem* tag = nullptr;
        QGraphicsSimpleTextItem* lru = nullptr;
        QGraphicsSimpleTextItem* valid = nullptr;
        QGraphicsSimpleTextItem* dirty = nullptr;
    };

    using CacheLine = std::map<unsigned, CacheWay>;

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
