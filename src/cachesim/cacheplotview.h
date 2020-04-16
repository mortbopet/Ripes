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

class CachePlotView : public QGraphicsView {
    Q_OBJECT

public:
    CachePlotView(QWidget* parent = nullptr);

    void setPlot(QChart* chart);

protected:
    void resizeEvent(QResizeEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

public slots:
    void keepCallout();
    void tooltip(QPointF point, bool state);

private:
    QPointF m_hoverPos;

    QGraphicsSimpleTextItem* m_coordX;
    QGraphicsSimpleTextItem* m_coordY;
    QChart* m_chart = nullptr;
    Callout* m_tooltip = nullptr;
    QMutex m_tooltipLock;
};

}  // namespace Ripes
