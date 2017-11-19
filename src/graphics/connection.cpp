#include "connection.h"
#include "shape.h"

#include <QPainter>

namespace Graphics {
static const float Pi = 3.1415926535;

Connection::Connection(Shape *source, QPointF sourcePoint, Shape *dest,
                       QPointF destPoint) {
  m_sourcePoint = mapFromItem(source, sourcePoint);
  m_destPoint = mapFromItem(dest, destPoint);
}

QRectF Connection::boundingRect() const {
  qreal penWidth = 1;
  qreal extra = (penWidth + m_arrowSize) / 2.0;

  return QRectF(m_sourcePoint, QSizeF(m_destPoint.x() - m_sourcePoint.x(),
                                      m_destPoint.y() - m_sourcePoint.y()))
      .normalized()
      .adjusted(-extra, -extra, extra, extra);
}

void Connection::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) {
  QLineF line(m_sourcePoint, m_destPoint);

  painter->setPen(
      QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter->drawLine(line);

  // Draw destination arrow
  double angle = ::acos(line.dx() / line.length());
  QPointF destArrowP1 =
      m_destPoint + QPointF(sin(angle - Pi / 3) * m_arrowSize,
                            cos(angle - Pi / 3) * m_arrowSize);
  QPointF destArrowP2 =
      m_destPoint + QPointF(sin(angle - Pi + Pi / 3) * m_arrowSize,
                            cos(angle - Pi + Pi / 3) * m_arrowSize);
  painter->setBrush(Qt::black);
  painter->drawPolygon(QPolygonF()
                       << m_destPoint << destArrowP1 << destArrowP2);
}

}  // namespace Graphics
