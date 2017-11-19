#include "connection.h"
#include "shape.h"

#include <QPainter>

namespace Graphics {
static const float Pi = 3.1415926535;

Connection::Connection(Shape *source, QPointF *sourcePoint, Shape *dest,
                       QPointF *destPoint)
    : m_source(source),
      m_dest(dest),
      m_sourcePointPtr(sourcePoint),
      m_destPointPtr(destPoint) {}

QRectF Connection::boundingRect() const {
    qreal penWidth = 1;
    qreal extra = (penWidth + m_arrowSize) / 2.0;

    QPointF sourcePoint = mapFromItem(m_source, *m_sourcePointPtr);
    QPointF destPoint = mapFromItem(m_dest, *m_destPointPtr);

    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                      destPoint.y() - sourcePoint.y()))
      .normalized()
      .adjusted(-extra, -extra, extra, extra);
}

QPair<QPointF, QPointF> Connection::getPoints() const {
    return QPair<QPointF, QPointF>(mapFromItem(m_source, *m_sourcePointPtr),
                                   mapFromItem(m_dest, *m_destPointPtr));
}

void Connection::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) {
    QPointF sourcePoint = mapFromItem(m_source, *m_sourcePointPtr);
    QPointF destPoint = mapFromItem(m_dest, *m_destPointPtr);

    QLineF line(sourcePoint, destPoint);

    painter->setPen(
      QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);

    // Draw destination arrow
    double angle = ::acos(line.dx() / line.length());
    QPointF destArrowP1 =
      destPoint + QPointF(sin(angle - Pi / 3) * m_arrowSize,
                          cos(angle - Pi / 3) * m_arrowSize);
    QPointF destArrowP2 =
      destPoint + QPointF(sin(angle - Pi + Pi / 3) * m_arrowSize,
                          cos(angle - Pi + Pi / 3) * m_arrowSize);
    painter->setBrush(Qt::black);
    painter->drawPolygon(QPolygonF()
                         << destPoint << destArrowP1 << destArrowP2);
}

}  // namespace Graphics
