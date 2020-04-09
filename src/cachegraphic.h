#pragma once

#include <QFont>
#include <QGraphicsItem>
#include <QObject>
#include "cachebase.h"

namespace Ripes {

class CacheGraphic : public QGraphicsObject {
public:
    CacheGraphic(CacheBase& cache);

    QRectF boundingRect() const override {}

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override {}

    void lineChanged(uint32_t address, unsigned block);

    /**
     * @brief cacheParametersChanged
     * Recalculates and redraws the graphic based on the current cache parameters
     */
    void cacheParametersChanged();

private:
    void drawText(const QString& text, qreal x, qreal y);

    QFont m_font = QFont("Roboto", 12);
    CacheBase& m_cache;
};

}  // namespace Ripes
