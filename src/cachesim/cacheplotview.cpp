#include "cacheplotview.h"

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsTextItem>
#include "callout.h"

void constrainInPlotRange(QPointF& pos, QChart* chart) {
    const auto plotValue = chart->mapToValue(pos);

    const QPointF chartBottomLeft = chart->mapToPosition({0, 0});
    const QRectF cbr = chart->boundingRect();
    const QPointF chartTopRight = QPointF{chartBottomLeft.x() + cbr.width(), chartBottomLeft.y() + cbr.height()};

    pos.rx() = plotValue.x() < 0 ? chartBottomLeft.x() : pos.x();
    pos.ry() = plotValue.y() < 0 ? chartBottomLeft.y() : pos.y();

    pos.rx() = pos.x() > chartTopRight.x() ? chartTopRight.x() : pos.x();
    pos.ry() = pos.y() > chartTopRight.y() ? chartTopRight.y() : pos.y();
}

namespace Ripes {

CachePlotView::CachePlotView(QWidget* parent) : QGraphicsView(new QGraphicsScene, parent) {
    setMouseTracking(true);
}

void CachePlotView::setPlot(QChart* chart) {
    // Clear any previous plot or plot markers
    scene()->clear();
    m_tooltip = nullptr;
    m_chart = chart;
    scene()->addItem(chart);

    for (auto* series : chart->series()) {
        if (auto* lineSeries = dynamic_cast<QLineSeries*>(series)) {
            connect(lineSeries, &QLineSeries::clicked, this, &CachePlotView::keepCallout);
            connect(lineSeries, &QLineSeries::hovered, this, &CachePlotView::tooltip);
        }
    }

    m_coordX = new QGraphicsSimpleTextItem();
    scene()->addItem(m_coordX);
    m_coordY = new QGraphicsSimpleTextItem();
    scene()->addItem(m_coordY);
    m_coordX->setText("X: ");
    m_coordY->setText("Y: ");
    m_coordX->setPos(m_chart->size().width() / 2 - 50, m_chart->size().height());
    m_coordY->setPos(m_chart->size().width() / 2 + 50, m_chart->size().height());
}

void CachePlotView::resizeEvent(QResizeEvent* event) {
    if (scene() && m_chart) {
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
        m_chart->resize(event->size());
        m_coordX->setPos(m_chart->size().width() / 2 - 50, m_chart->size().height() - 20);
        m_coordY->setPos(m_chart->size().width() / 2 + 50, m_chart->size().height() - 20);
        for (auto* item : scene()->items()) {
            if (auto* callout = dynamic_cast<Callout*>(item)) {
                callout->updateGeometry();
            }
        }
    }
    QGraphicsView::resizeEvent(event);
}

void CachePlotView::mouseMoveEvent(QMouseEvent* event) {
    m_hoverPos = event->pos();
    constrainInPlotRange(m_hoverPos, m_chart);

    m_coordX->setText(QString("X: %1").arg(m_chart->mapToValue(m_hoverPos).x()));
    m_coordY->setText(QString("Y: %1").arg(m_chart->mapToValue(m_hoverPos).y()));
    QGraphicsView::mouseMoveEvent(event);
}

void CachePlotView::keepCallout() {
    m_tooltipLock.lock();
    m_tooltip = nullptr;
    m_tooltipLock.unlock();
}

void CachePlotView::deleteCallout(Callout* callout) {
    if (m_tooltip == callout) {
        m_tooltip = nullptr;
    }
    delete callout;
}

void CachePlotView::tooltip(QPointF point, bool state) {
    m_tooltipLock.lock();
    if (m_tooltip == nullptr)
        m_tooltip = new Callout(this, m_chart);

    if (state) {
        m_tooltip->setText(QString("X: %1 \nY: %2 ").arg(point.x()).arg(point.y()));
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    } else {
        m_tooltip->hide();
    }
    m_tooltipLock.unlock();
}
}  // namespace Ripes
