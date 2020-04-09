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
    void updateHighlighting(bool active);
    void drawText(const QString& text, qreal x, qreal y);

    QFont m_font = QFont("Roboto", 12);
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

    // Data structure modelling the cache; keeping graphics text items for each entry
    using CacheWay = std::map<unsigned, QGraphicsSimpleTextItem*>;
    using CacheLine = std::map<unsigned, CacheWay>;
    std::map<unsigned, CacheLine> m_cacheTextItems;
};

}  // namespace Ripes
