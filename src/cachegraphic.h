#pragma once

#include <QFont>
#include <QGraphicsItem>
#include <QObject>
#include <memory>
#include "cachebase.h"

namespace Ripes {

class CacheGraphic : public QGraphicsObject {
public:
    CacheGraphic(CacheBase& cache);

    QRectF boundingRect() const override {}

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override {}

    void dataChanged(uint32_t address);

    /**
     * @brief cacheParametersChanged
     * Recalculates and redraws the graphic based on the current cache parameters
     */
    void cacheParametersChanged();

private:
    /**
     * @brief initializeControlBits
     * Constructs all of the "Valid" and "LRU" text items within the cache
     */
    void initializeControlBits();
    void updateHighlighting(bool active);
    void drawText(const QString& text, qreal x, qreal y);

    QFont m_font = QFont("Inconsolata", 12);
    CacheBase& m_cache;

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
    qreal m_lruWidth = 0;

    // Data structure modelling the cache; keeping graphics text items for each entry
    struct CacheWay {
        std::map<unsigned, QGraphicsSimpleTextItem*> blocks;
        QGraphicsSimpleTextItem* tag = nullptr;
        QGraphicsSimpleTextItem* lru = nullptr;
        QGraphicsSimpleTextItem* valid = nullptr;
    };

    using CacheLine = std::map<unsigned, CacheWay>;
    std::map<unsigned, CacheLine> m_cacheTextItems;
};

}  // namespace Ripes
