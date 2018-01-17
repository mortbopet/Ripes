#include "connection.h"
#include <cmath>
#include "shape.h"

#include <QFontMetrics>
#include <QPainter>

namespace Graphics {
static constexpr float Pi = 3.1415926535;

namespace {
inline double pointDistance(QPointF a, QPointF b) {
    return std::sqrt(std::pow(a.x() - b.x(), 2) + std::pow(a.y() - b.y(), 2));
}
}  // namespace
using namespace std;

Connection::Connection(Shape* source, QPointF* sourcePoint, Shape* dest, QPointF* destPoint)
    : QObject(), m_source(source), m_sourcePointPtr(sourcePoint) {
    m_dests << QPair<Shape*, QPointF*>(dest, destPoint);
    m_label.m_source = source;
    m_label.m_drawPos = sourcePoint;
    update();
}

Connection::Connection(Shape* source, QPointF* sourcePoint, QList<PointPair> dests)
    : QObject(), m_source(source), m_sourcePointPtr(sourcePoint) {
    m_dests = dests;
    m_label.m_source = source;
    m_label.m_drawPos = sourcePoint;
    update();
}

QRectF Connection::boundingRect() const {
    const static double pad = 5;  // pad around each edge
    double left = 0;
    double top = 0;
    double bot = 0;
    double right = 0;
    // Iterate through all points in the connection line to find bounding rect
    for (const auto& pointVec : m_polyLines) {
        for (const auto& point : pointVec) {
            left = point.x() < left ? point.x() : left;
            right = point.x() > right ? point.x() : right;
            top = point.y() < top ? point.y() : top;
            bot = point.y() > bot ? point.y() : bot;
        }
    }
    return QRectF(left - pad, top - pad, right - left + pad, bot - top + pad);
}

QPair<QPointF, QPointF> Connection::getPoints() const {
    return QPair<QPointF, QPointF>(mapFromItem(m_source, *m_sourcePointPtr),
                                   mapFromItem(m_dests[0].first, *m_dests[0].second));
}

void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    QPointF sourcePoint = mapFromItem(m_source, *m_sourcePointPtr);

    // sort destinations
    if (m_dests.size() > 1) {
        std::sort(m_dests.begin(), m_dests.end(), [=](const PointPair& a, const PointPair& b) {
            return pointDistance(sourcePoint, mapFromItem(a.first, *a.second)) <
                   pointDistance(sourcePoint, mapFromItem(b.first, *b.second));
        });
    }
    QPointF destPoint = mapFromItem(m_dests[0].first, *m_dests[0].second);

    // Number of kinks depends on the directon of the line. If its left to right,
    // óne kink will be present, if it is right to left, 3 kinks will be present
    bool dir = (destPoint.x() - sourcePoint.x()) > 0 ? true : false;

    // Generate polygon from calculated points
    m_polyLines.clear();
    if (dir) {
        // Feed forward connection
        QList<QPointF> sourcePoints = QList<QPointF>() << sourcePoint;
        for (int i = 0; i < m_dests.length(); i++) {
            QVector<QPointF> polyLine;
            QPointF dest = mapFromItem(m_dests[i].first, *m_dests[i].second);

            // Get bias for the given connection
            int bias = m_kinkBiases.size() > i ? m_kinkBiases[i] : 0;

            // Find closest available source point
            QPointF source = sourcePoint;
            for (const auto& p : sourcePoints) {
                source = pointDistance(dest, p) < pointDistance(dest, source) ? p : source;
            }
            polyLine << source;

            int xDiff = dest.x() - source.x();
            if (i == 0) {
                // do kink for first line
                if (m_dir == Direction::east) {
                    QPointF point(source.x() + xDiff / 2 + bias, source.y());
                    QPointF point2(point.x(), dest.y());
                    if (m_pointAtFirstKink) {
                        // Draw kink points
                        painter->setPen(QPen(Qt::black, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        painter->drawPoint(point);
                    }
                    sourcePoints << point << point2;
                    polyLine << point << point2;
                } else {
                    QPointF point(dest.x(), source.y());
                    if (m_pointAtFirstKink) {
                        // Draw kink points
                        painter->setPen(QPen(Qt::black, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        painter->drawPoint(point);
                    }
                    sourcePoints << point;
                    polyLine << point;
                }

            } else {
                QPointF point(source.x(), dest.y());
                sourcePoints << point;
                polyLine << point;
            }
            polyLine << dest;
            m_polyLines.append(polyLine);
        }
    } else {
        // Feedback connection
        Q_ASSERT(m_kinkBiases.length() > 0);
        QList<QPointF> sourcePoints = QList<QPointF>() << sourcePoint;
        for (int i = 0; i < m_dests.length(); i++) {
            QVector<QPointF> polyLine;
            QPointF dest = mapFromItem(m_dests[i].first, *m_dests[i].second);

            // Find closest available source point
            QPointF source = sourcePoint;
            for (const auto& p : sourcePoints) {
                source = pointDistance(dest, p) < pointDistance(dest, source) ? p : source;
            }
            polyLine << source;

            if (i == 0) {
                // do kink for first line
                QPointF point(source.x() + m_sourceStubLen, source.y());
                QPointF point2(point.x(), m_kinkBiases[i] + point.y());
                if (m_dir == Direction::east) {
                    QPointF point3(dest.x() - m_destStubLen, point2.y());
                    QPointF point4(point3.x(), dest.y());
                    sourcePoints << point << point2 << point3;
                    if (!m_invalidDestSourcePoints.contains(i))
                        sourcePoints << point4;
                    polyLine << point << point2 << point3 << point4;
                } else {
                    QPointF point4(dest.x(), point2.y());
                    sourcePoints << point << point2;
                    if (!m_invalidDestSourcePoints.contains(i))
                        sourcePoints << point4;
                    polyLine << point << point2 << point4;
                }
            } else {
                QPointF point(dest.x() - m_destStubLen, source.y());
                QPointF point2(point.x(), dest.y());
                sourcePoints << point;
                if (!m_invalidDestSourcePoints.contains(i))
                    sourcePoints << point2;
                polyLine << point << point2;
            }
            polyLine << dest;
            m_polyLines.append(polyLine);
        }
    }

    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::black);
    // painter->setCompositionMode(QPainter::CompositionMode_DestinationOver);

    for (const auto& line : m_polyLines) {
        // Draw connection polyline

        painter->drawPolyline(QPolygonF(line));
        // Draw destination arrows - angles are a bit strange
        qreal angle;
        switch (m_dir) {
            case Direction::east:
                angle = 0;
                break;
            case Direction::north:
                angle = 2 * Pi / 4;
                break;
            case Direction::west:
                angle = Pi / 2;
                break;
            case Direction::south:
                angle = -2 * Pi / 4;
                break;
        }

        QPointF destArrowP1 =
            *(line.end() - 1) + QPointF(std::sin(angle - Pi / 3) * m_arrowSize, std::cos(angle - Pi / 3) * m_arrowSize);
        QPointF destArrowP2 = *(line.end() - 1) + QPointF(std::sin(angle - Pi + Pi / 3) * m_arrowSize,
                                                          std::cos(angle - Pi + Pi / 3) * m_arrowSize);

        painter->drawPolygon(QPolygonF() << *(line.end() - 1) << destArrowP1 << destArrowP2);
    }

    // Draw kink points
    painter->setPen(QPen(Qt::black, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    // If a point is present in two lines, it is a kink point - generate a map for this, and draw the resulting points
    QMap<QPointF, int> pointMap;
    for (const auto& line : m_polyLines) {
        for (const auto& point : line) {
            pointMap[point]++;
        }
    }

    auto i = pointMap.begin();
    while (i != pointMap.end()) {
        if (i.value() > 1) {
            painter->drawPoint(i.key());
        }
        i++;
    }

    // Debugging: draw bounding rect
    /*
    painter->setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::transparent);
    painter->drawRect(boundingRect());
*/
    update();
}

void Connection::addLabelToScene() {
    // Must be called after connection has been added to a scene
    scene()->addItem(&m_label);
    update();
}

// --------- label ------------
void Label::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_showValue && m_signal != nullptr) {
        // Draw label (connection value)
        auto text = QString("0x%1").arg(QString().setNum(m_signal->getValue(), 16));
        QPointF pos = mapFromItem(m_source, *m_drawPos);
        QFontMetrics metric = QFontMetrics(QFont(text));
        QRectF textRect = metric.boundingRect(text);
        textRect.moveTo(pos);
        textRect.translate(0, -textRect.height());
        textRect.adjust(-5, 0, 5, 5);
        painter->fillRect(textRect, Qt::white);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::black, 1));
        painter->drawRect(textRect);
        painter->setFont(QFont());
        painter->drawText(pos, text);
    }
}

QRectF Label::boundingRect() const {
    if (m_signal != nullptr) {
        auto text = QString("0x%1").arg(QString().setNum(m_signal->getValue(), 16));
        QPointF pos = mapFromItem(m_source, *m_drawPos);
        QFontMetrics metric = QFontMetrics(QFont());
        QRectF textRect = metric.boundingRect(text);
        textRect.moveTo(pos);
        textRect.translate(0, -textRect.height());
        textRect.adjust(-5, 0, 5, 5);
        return textRect;
    }
    return QRectF();
}

}  // namespace Graphics
