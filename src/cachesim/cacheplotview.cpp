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

MarkerObjects::MarkerObjects(QObject* parent, QChart* chart, const QLineSeries* _series) : QObject(parent) {
    marker = new ChartLineMarker(chart, _series);
    coordX = new QGraphicsSimpleTextItem(chart);
    coordY = new QGraphicsSimpleTextItem(chart);
    label = new QGraphicsSimpleTextItem(chart);
    label->setText(_series->name() + ":");
    series = _series;
    coordX->setText("Cycle: ");
    coordY->setText("Value: ");

    connect(marker, &ChartLineMarker::snappedMarkerChanged,
            [=] { this->updateCoordinateValues(marker->getMarkerValuePos()); });
}

void MarkerObjects::clear() {
    delete marker;
    delete coordX;
    delete coordY;
    delete label;
}

void MarkerObjects::updateCoordinateValues(const QPointF& pos) {
    coordX->setText(QString("Cycle: %1").arg(pos.x()));
    coordY->setText(QString("Value: %1").arg(pos.y()));
}

CachePlotView::CachePlotView(QWidget* parent) : QGraphicsView(new QGraphicsScene, parent) {
    setMouseTracking(true);
}

void CachePlotView::enterEvent(QEvent* event) {
    for (const auto& marker : m_markers) {
        marker->marker->show();
    }

    QGraphicsView::enterEvent(event);
}

void CachePlotView::leaveEvent(QEvent* event) {
    for (const auto& marker : m_markers) {
        marker->marker->hide();
    }
    QGraphicsView::leaveEvent(event);
}

QPixmap CachePlotView::getPlotPixmap() {
    QVector<QGraphicsItem*> itemsToHide;
    for (const auto& marker : m_markers) {
        itemsToHide << marker->marker;
        itemsToHide << marker->coordX;
        itemsToHide << marker->coordY;
        itemsToHide << marker->label;
    }

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

void CachePlotView::hideSeriesMarker(const QLineSeries* series) {
    auto marker =
        std::find_if(m_markers.begin(), m_markers.end(), [=](const auto& mob) { return mob->series == series; });
    Q_ASSERT(marker != m_markers.end());
    marker->get()->clear();
    m_markers.erase(marker);
    resizeObjects(scene()->sceneRect().size());
}

void CachePlotView::showSeriesMarker(const QLineSeries* series) {
    m_markers.emplace_back(std::make_unique<MarkerObjects>(this, m_chart, series));
    resizeObjects(scene()->sceneRect().size());
}

void CachePlotView::setPlot(QChart* chart) {
    if (m_chart) {
        // Propagate the previous chart size to the new plot
        resizeObjects(scene()->sceneRect().size());
    }

    // Clear any previous plot or plot markers
    scene()->clear();
    m_tooltip = nullptr;
    m_markers.clear();
    m_chart = chart;
    scene()->addItem(chart);

    for (auto* series : chart->series()) {
        if (auto* lineSeries = dynamic_cast<QLineSeries*>(series)) {
            connect(lineSeries, &QLineSeries::clicked, this, &CachePlotView::keepCallout);
            connect(lineSeries, &QLineSeries::hovered, this, &CachePlotView::tooltip);
        }
    }

    resizeObjects(m_chart->size());
}

void CachePlotView::resizeObjects(const QSizeF& size) {
    int ySpaceForMarkers = 0;
    if (m_markers.size() > 0) {
        ySpaceForMarkers = m_markers.size() * m_markers.at(0)->coordX->boundingRect().height();
    }

    scene()->setSceneRect(QRectF(QPoint(0, 0), size));
    m_chart->resize(size - QSizeF(0, ySpaceForMarkers));
    for (auto* item : scene()->items()) {
        if (auto* callout = dynamic_cast<Callout*>(item)) {
            callout->updateGeometry();
        }
    }

    for (unsigned i = 0; i < m_markers.size(); i++) {
        auto& marker = m_markers.at(i);
        int yOffset = 0;
        if (i > 0) {
            yOffset = m_markers.at(i - 1)->coordX->boundingRect().height() * i;
        }
        const int y = yOffset + m_chart->size().height() - 20;
        marker->label->setPos(m_chart->size().width() / 4 - marker->label->boundingRect().width(), y);
        marker->coordX->setPos(m_chart->size().width() / 3, y);
        marker->coordY->setPos(m_chart->size().width() / 2, y);
        marker->marker->updateLines();
    }
}

void CachePlotView::resizeEvent(QResizeEvent* event) {
    if (scene() && m_chart) {
        resizeObjects(event->size());
    }
    QGraphicsView::resizeEvent(event);
}

void CachePlotView::mouseMoveEvent(QMouseEvent* event) {
    m_hoverPos = event->pos();
    constrainInPlotRange(m_hoverPos, m_chart);

    if (m_markers.size() > 0) {
        for (const auto& marker : m_markers) {
            marker->marker->move(m_hoverPos);
        }
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
