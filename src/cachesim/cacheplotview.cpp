#include "cacheplotview.h"

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsTextItem>

#include "callout.h"
#include "chartlinemarker.h"

void constrainInPlotRange(QPointF& pos, QChart* chart) {
    const auto plotValue = chart->mapToValue(pos);

    const QValueAxis* axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    const QValueAxis* axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    const QPointF chartBottomLeft = chart->mapToPosition({axisX->min(), axisY->min()});
    const QPointF chartTopRight = chart->mapToPosition({axisX->max(), axisY->max()});

    pos.rx() = plotValue.x() < axisX->min() ? chartBottomLeft.x() : pos.x();
    pos.ry() = plotValue.y() < axisY->min() ? chartBottomLeft.y() : pos.y();

    pos.rx() = plotValue.x() > axisX->max() ? chartTopRight.x() : pos.x();
    pos.ry() = plotValue.y() > axisY->max() ? chartTopRight.y() : pos.y();
}

namespace Ripes {

CachePlotView::CachePlotView(QWidget* parent) : QGraphicsView(new QGraphicsScene, parent) {
    setMouseTracking(true);
}

void CachePlotView::enterEvent(QEvent* event) {
    if (m_marker && m_crosshairEnabled) {
        m_marker->show();
    }
    QGraphicsView::enterEvent(event);
}

void CachePlotView::leaveEvent(QEvent* event) {
    if (m_marker) {
        m_marker->hide();
    }
    QGraphicsView::leaveEvent(event);
}

QPixmap CachePlotView::getPlotPixmap() {
    std::vector<QGraphicsItem*> itemsToHide = {m_coordX, m_coordY, m_marker};

    for (const auto& i : itemsToHide) {
        if (i)
            i->hide();
    }

    QPixmap p = grab();

    for (const auto& i : itemsToHide) {
        if (i)
            i->show();
    }

    return p;
};

void CachePlotView::setPlot(QChart* chart) {
    if (m_chart) {
        // Propagate the previous chart size to the new plot
        chart->resize(m_chart->size());
    }

    // Clear any previous plot or plot markers
    scene()->clear();
    m_tooltip = nullptr;
    m_marker = nullptr;
    m_chart = chart;
    scene()->addItem(chart);

    for (auto* series : chart->series()) {
        if (auto* lineSeries = dynamic_cast<QLineSeries*>(series)) {
            connect(lineSeries, &QLineSeries::clicked, this, &CachePlotView::keepCallout);
            connect(lineSeries, &QLineSeries::hovered, this, &CachePlotView::tooltip);
        }
    }

    if (m_chart->series().size() > 0) {
        if (auto* lineSeries = dynamic_cast<QLineSeries*>(m_chart->series()[0])) {
            m_marker = new ChartLineMarker(m_chart, lineSeries);
            connect(m_marker, &ChartLineMarker::snappedMarkerChanged,
                    [=] { this->updateCoordinateValues(m_marker->getMarkerValuePos()); });

            m_marker->setVisible(hasFocus());
        }
    }

    m_coordX = new QGraphicsSimpleTextItem();
    scene()->addItem(m_coordX);
    m_coordY = new QGraphicsSimpleTextItem();
    scene()->addItem(m_coordY);
    m_coordX->setText("Cycle: ");
    m_coordY->setText("Value: ");
    m_coordX->setPos(m_chart->size().width() / 2 - 50, m_chart->size().height());
    m_coordY->setPos(m_chart->size().width() / 2 + 50, m_chart->size().height());

    resizeObjects(m_chart->size());
}

void CachePlotView::resizeObjects(const QSizeF& size) {
    scene()->setSceneRect(QRectF(QPoint(0, 0), size));
    m_chart->resize(size);
    m_coordX->setPos(m_chart->size().width() / 2 - 50, m_chart->size().height() - 20);
    m_coordY->setPos(m_chart->size().width() / 2 + 50, m_chart->size().height() - 20);
    for (auto* item : scene()->items()) {
        if (auto* callout = dynamic_cast<Callout*>(item)) {
            callout->updateGeometry();
        }
        if (m_crosshairEnabled && m_marker) {
            m_marker->updateLines();
        }
    }
}

void CachePlotView::resizeEvent(QResizeEvent* event) {
    if (scene() && m_chart) {
        resizeObjects(event->size());
    }
    QGraphicsView::resizeEvent(event);
}

void CachePlotView::updateCoordinateValues(const QPointF& pos) {
    m_coordX->setText(QString("Cycle: %1").arg(pos.x()));
    m_coordY->setText(QString("Value: %1").arg(pos.y()));
}

void CachePlotView::mouseMoveEvent(QMouseEvent* event) {
    m_hoverPos = event->pos();
    constrainInPlotRange(m_hoverPos, m_chart);

    if (m_crosshairEnabled && m_marker) {
        m_marker->move(m_hoverPos);
    } else {
        // Only update coordinate values if the marker is not currently snapping. If it is snapping, the marker itself
        // will emit signals with the snapped marker position.
        updateCoordinateValues(m_chart->mapToValue(m_hoverPos));
    }
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

void CachePlotView::enableCrosshair(bool enabled) {
    m_crosshairEnabled = enabled;
}

void CachePlotView::tooltip(QPointF point, bool state) {
    m_tooltipLock.lock();
    if (m_tooltip == nullptr)
        m_tooltip = new Callout(this, m_chart);

    if (state) {
        m_tooltip->setText(QString("Cycle: %1 \nValue: %2 ").arg(point.x()).arg(point.y()));
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
