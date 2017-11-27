#include "connection.h"
#include "shape.h"

#include <QFontMetrics>
#include <QPainter>

namespace Graphics {
static constexpr float Pi = 3.1415926535;

Connection::Connection(Shape* source, QPointF* sourcePoint, Shape* dest, QPointF* destPoint)
    : QObject(), m_source(source), m_sourcePointPtr(sourcePoint) {
    m_dests << QPair<Shape*, QPointF*>(dest,destPoint);
    m_label.m_source  = source;
    m_label.m_drawPos = sourcePoint;
}

Connection::Connection(Shape* source, QPointF* sourcePoint, QList<QPair<Shape*, QPointF*>> dests) : QObject(), m_source(source),m_sourcePointPtr(sourcePoint){
    m_dests = dests;
    m_label.m_source = source;
    m_label.m_drawPos = sourcePoint;
}

QRectF Connection::boundingRect() const {
    qreal   penWidth    = 1;
    qreal   extra       = (penWidth + m_arrowSize) / 2.0;
    QPointF sourcePoint = mapFromItem(m_source, *m_sourcePointPtr);
    QPointF destPoint   = mapFromItem(m_dests[0].first, *m_dests[0].second);

    if ((destPoint.x() - sourcePoint.x()) > 0) {
        return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(), destPoint.y() - sourcePoint.y()))
            .normalized()
            .adjusted(-extra, -extra, extra, extra);
    } else if (m_kinkBiases.size() > 0) {
        return QRectF(QPointF(destPoint.x() - m_destStubLen, sourcePoint.y() + m_feedbackDir * m_kinkBiases[0]),
                      QPointF(sourcePoint.x() + m_sourceStubLen, destPoint.y()))
            .normalized()
            .adjusted(-extra, -extra, extra, extra);
    }
    return QRectF();
}

QPair<QPointF, QPointF> Connection::getPoints() const {
    return QPair<QPointF, QPointF>(mapFromItem(m_source, *m_sourcePointPtr), mapFromItem(m_dests[0].first, *m_dests[0].second));
}

void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QPointF sourcePoint = mapFromItem(m_source, *m_sourcePointPtr);
    QPointF destPoint   = mapFromItem(m_dests[0].first, *m_dests[0].second);

    // Number of kinks depends on the directon of the line. If its left to right,
    // óne kink will be present, if it is right to left, 3 kinks will be present
    bool dir = (destPoint.x() - sourcePoint.x()) > 0 ? true : false;

    // Generate polygon from calculated points
    QVector<QPointF> polyLine;
    polyLine << sourcePoint;
    if (dir) {
        // Feed forward connection
        int     bias  = m_kinkBiases.size() > 0 ? m_kinkBiases[0] : 0;
        int     xDiff = destPoint.x() - sourcePoint.x();
        QPointF point(sourcePoint.x() + xDiff / 2 + bias, sourcePoint.y());
        QPointF point2(point.x(), destPoint.y());
        polyLine << point << point2;
    } else {
        // Feedback connection
        // 1. traverse to the right of the input
        if (m_kinkBiases.size() >= 1) {
            QPointF point(sourcePoint.x() + m_sourceStubLen, sourcePoint.y());
            QPointF point2(point.x(), point.y() + m_feedbackDir * m_kinkBiases[0]);
            QPointF point3(destPoint.x() - m_destStubLen, point2.y());
            QPointF point4(point3.x(), destPoint.y());
            polyLine << point << point2 << point3 << point4;
        }
    }
    polyLine << destPoint;

    // Draw connection polyline
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawPolyline(QPolygonF(polyLine));

    // Draw destination arrow
    qreal   angle       = 0;
    QPointF destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * m_arrowSize, cos(angle - Pi / 3) * m_arrowSize);
    QPointF destArrowP2 =
        destPoint + QPointF(sin(angle - Pi + Pi / 3) * m_arrowSize, cos(angle - Pi + Pi / 3) * m_arrowSize);
    painter->setBrush(Qt::black);
    painter->drawPolygon(QPolygonF() << destPoint << destArrowP1 << destArrowP2);

    // Draw kink points
    painter->setPen(QPen(Qt::black, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (const auto& point : m_kinkPoints) {
        painter->drawPoint(polyLine[point + 1]);
    }
}

void Connection::setValue(uint32_t value) {
    QString text;
    if (value < 0b1111) {
        // Draw in binary notation
        text = QString("0b%1").arg(QString().setNum(value, 2));
    } else {
        // Draw in hex notation
        text = QString("0x%1").arg(QString().setNum(value, 16));
    }
    m_label.setText(text);
    m_label.setToolTip(text);
}

void Connection::addLabelToScene() {
    // Must be called after connection has been added to a scene
    scene()->addItem(&m_label);
}

// --------- label ------------
void Label::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    if (m_showValue) {
        // Draw label (connection value)
        QPointF      pos      = mapFromItem(m_source, *m_drawPos);
        QFontMetrics metric   = QFontMetrics(QFont());
        QRectF       textRect = metric.boundingRect(m_text);
        textRect.moveTo(pos);
        textRect.translate(0, -textRect.height());
        textRect.adjust(-5, 0, 5, 5);
        painter->fillRect(textRect, Qt::white);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::black, 1));
        painter->drawRect(textRect);
        painter->setFont(QFont());
        painter->drawText(pos, m_text);
    }
}

QRectF Label::boundingRect() const {
    QPointF      pos      = mapFromItem(m_source, *m_drawPos);
    QFontMetrics metric   = QFontMetrics(QFont());
    QRectF       textRect = metric.boundingRect(m_text);
    textRect.moveTo(pos);
    textRect.translate(0, -textRect.height());
    textRect.adjust(-5, 0, 5, 5);
    return textRect;
}

}  // namespace Graphics
