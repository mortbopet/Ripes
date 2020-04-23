#include "chartlinemarker.h"

#include <QtCharts/QValueAxis>
#include <algorithm>

namespace Ripes {

ChartLineMarker::ChartLineMarker(QChart* parent, QLineSeries* series)
    : QGraphicsObject(parent), m_chart(parent), m_series(series) {
    m_hzLine = new QGraphicsLineItem(this);
    m_vtLine = new QGraphicsLineItem(this);

    setFlag(ItemHasNoContents, true);
}

QRectF ChartLineMarker::boundingRect() const {
    return childrenBoundingRect();
}

void ChartLineMarker::snapToLine(bool enabled) {
    if (enabled ^ m_snapToLine) {
        m_snapToLine = enabled;
        move(m_center);
    }
}

void ChartLineMarker::updateLines() {
    const QValueAxis* axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());
    const QValueAxis* axisX = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Horizontal).first());

    QLineF hzLine, vtLine;

    const QPointF chartBottomLeft = m_chart->mapToPosition({0, 0});
    const QPointF chartTopRight = m_chart->mapToPosition({axisX->max(), axisY->max()});

    hzLine.setP1(QPointF(chartBottomLeft.x(), m_center.y()));
    hzLine.setP2(QPointF(chartTopRight.x(), m_center.y()));

    vtLine.setP1(QPointF(m_center.x(), chartBottomLeft.y()));
    vtLine.setP2(QPointF(m_center.x(), chartTopRight.y()));

    m_hzLine->setLine(hzLine);
    m_vtLine->setLine(vtLine);
}

QPointF ChartLineMarker::getMarkerValuePos() const {
    return m_chart->mapToValue(m_center);
}
QPointF ChartLineMarker::getMarkerPos() const {
    return m_center;
}

void ChartLineMarker::move(const QPointF& center) {
    if (m_snapToLine) {
        const QValueAxis* axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());
        const QValueAxis* axisX = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Horizontal).first());

        // Note: we assume that the points are in a sorted order!
        const auto& points = m_series->pointsVector();
        const QPointF chartPos = m_chart->mapToValue(center);
        auto iter = std::lower_bound(points.begin(), points.end(), chartPos.x(),
                                     [=](const QPointF& lhs, const qreal& closestX) { return lhs.x() < closestX; });

        if (iter != points.begin()) {
            // we always want to select a point which is less than chartPos.x such that the marker starts on the closest
            // step backwards in the X direction, rather than the closest step forwards in the X direction.
            iter--;
        }
        // Reversing the iterator might have pushes the marker out of bounds with the plot. If so, do not decrement the
        // iterator
        if (iter->x() < axisX->min()) {
            iter++;
        }

        auto& pointInSeries = *iter;
        QPointF pointInScene = m_chart->mapToPosition(pointInSeries);

        const bool markerChanged = m_center != pointInScene;
        m_center = pointInScene;
        if (markerChanged) {
            emit snappedMarkerChanged();
        }

    } else {
        m_center = center;
    }
    updateLines();
}

}  // namespace Ripes
