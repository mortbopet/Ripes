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
QT_CHARTS_END_NAMESPACE

class Callout;

QT_CHARTS_USE_NAMESPACE

namespace Ripes {
class ChartLineMarker;

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
    void resizeEvent(QResizeEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

public slots:
    void keepCallout();
    void deleteCallout(Callout* callout);
    void tooltip(QPointF point, bool state);
    void enableCrosshair(bool enabled);

private:
    void updateCoordinateValues(const QPointF& pos);
    void resizeObjects(const QSizeF& size);

    QPointF m_hoverPos;

    QGraphicsSimpleTextItem* m_coordX;
    QGraphicsSimpleTextItem* m_coordY;
    QChart* m_chart = nullptr;
    Callout* m_tooltip = nullptr;
    ChartLineMarker* m_marker = nullptr;
    QMutex m_tooltipLock;

    bool m_crosshairEnabled = true;
};

}  // namespace Ripes
