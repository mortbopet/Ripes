#pragma once

#include <QMutex>
#include <QtCharts/QChartGlobal>
#include <QtWidgets/QGraphicsView>

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QMouseEvent;
class QResizeEvent;
QT_END_NAMESPACE

QT_CHARTS_BEGIN_NAMESPACE
class QChart;
class QLineSeries;
QT_CHARTS_END_NAMESPACE

class Callout;

QT_CHARTS_USE_NAMESPACE

namespace Ripes {
class ChartLineMarker;

class MarkerObjects : public QObject {
    Q_OBJECT

public:
    QGraphicsSimpleTextItem* coordX;
    QGraphicsSimpleTextItem* coordY;
    QGraphicsSimpleTextItem* label;
    ChartLineMarker* marker;
    const QLineSeries* series;
    MarkerObjects(QObject* parent, QChart* chart, const QLineSeries* series);
    void updateCoordinateValues(const QPointF& pos);
    void clear();
};

class CachePlotView : public QGraphicsView {
    Q_OBJECT

public:
    CachePlotView(QWidget* parent = nullptr);

    void setPlot(QChart* chart);

    /**
     * @brief getPlotPixmap
     * @returns a pixmap containing only the plot (ie. no coordinate values nor marker)
     */
    QPixmap getPlotPixmap();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

public slots:
    void keepCallout();
    void deleteCallout(Callout* callout);
    void tooltip(QPointF point, bool state);
    void showSeriesMarker(const QLineSeries* series);
    void hideSeriesMarker(const QLineSeries* series);

private:
    void resizeObjects(const QSizeF& size);

    QPointF m_hoverPos;

    std::vector<std::unique_ptr<MarkerObjects>> m_markers;

    QChart* m_chart = nullptr;
    Callout* m_tooltip = nullptr;
    QMutex m_tooltipLock;

    bool m_crosshairEnabled = true;
};

}  // namespace Ripes
