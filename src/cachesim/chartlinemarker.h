#pragma once

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>

#include <QGraphicsObject>

QT_CHARTS_USE_NAMESPACE

namespace Ripes {

class ChartLineMarker : public QGraphicsObject {
    Q_OBJECT
public:
    ChartLineMarker(QChart* parent, QLineSeries* series);
    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {}

    void move(const QPointF& center);
    void snapToLine(bool enabled);
    bool isSnapping() const { return m_snapToLine; }
    void updateLines();

    QPointF getMarkerValuePos() const;
    QPointF getMarkerPos() const;

signals:
    void snappedMarkerChanged();

private:
    /**
     * @brief m_snapToLine
     * If true, the center of the marker will snap to the closest x-axis point on the passed QLineSeries object.
     * Note: it is assumed that the points in the series are in a sorted order!
     */
    bool m_snapToLine = true;

    QPointF m_center;
    QChart* m_chart = nullptr;
    QLineSeries* m_series = nullptr;
    QGraphicsLineItem* m_hzLine = nullptr;
    QGraphicsLineItem* m_vtLine = nullptr;
};
}  // namespace Ripes
