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

void constrainInPlotRange(QPointF &pos, QChart *chart) {
  const auto plotValue = chart->mapToValue(pos);

  const QValueAxis *axisY =
      qobject_cast<QValueAxis *>(chart->axes(Qt::Vertical).constFirst());
  const QValueAxis *axisX =
      qobject_cast<QValueAxis *>(chart->axes(Qt::Horizontal).constFirst());
  const QPointF chartBottomLeft =
      chart->mapToPosition({axisX->min(), axisY->min()});
  const QPointF chartTopRight =
      chart->mapToPosition({axisX->max(), axisY->max()});

  pos.rx() = plotValue.x() < axisX->min() ? chartBottomLeft.x() : pos.x();
  pos.ry() = plotValue.y() < axisY->min() ? chartBottomLeft.y() : pos.y();

  pos.rx() = plotValue.x() > axisX->max() ? chartTopRight.x() : pos.x();
  pos.ry() = plotValue.y() > axisY->max() ? chartTopRight.y() : pos.y();
}

namespace Ripes {

CachePlotMarker::CachePlotMarker(QObject *parent, QChart *chart,
                                 QLineSeries *_series, const QString &_name)
    : QObject(parent), marker(new ChartLineMarker(chart, _series)),
      series(_series), name(_name) {
  connect(marker, &ChartLineMarker::snappedMarkerChanged, marker, [=] {
    this->updateCoordinateValues(marker->getMarkerValuePos(), true, false);
  });
  // Update initial marker text
  updateCoordinateValues(QPointF(), false, false);
}

void CachePlotMarker::clear() {
  if (marker != nullptr) {
    delete marker;
    marker = nullptr;
  }
}

void CachePlotMarker::updateCoordinateValues(const QPointF &pos, bool showValue,
                                             bool showCycles) {
  QString info = name;

  if (showValue) {
    info += QString(": %1").arg(pos.y());
  }
  if (showCycles) {
    info += QString(": %1").arg(pos.x());
  }
  series->setName(info);
}

CachePlotView::CachePlotView(QWidget *parent) : QGraphicsView(parent) {
  setMouseTracking(true);
}

void CachePlotView::enterEvent(QEnterEvent *event) {
  m_mouseInView = true;
  for (const auto &marker : m_markers) {
    marker->marker->show();
  }

  QGraphicsView::enterEvent(event);
}

void CachePlotView::leaveEvent(QEvent *event) {
  m_mouseInView = false;
  for (const auto &marker : m_markers) {
    marker->marker->hide();
    marker->updateCoordinateValues(QPointF(), false, false);
  }
  QGraphicsView::leaveEvent(event);
}

QPixmap CachePlotView::getPlotPixmap() {
  QVector<QGraphicsItem *> itemsToHide;
  for (const auto &marker : m_markers) {
    itemsToHide << marker->marker;
  }

  for (const auto &i : qAsConst(itemsToHide)) {
    if (i)
      i->hide();
  }

  QPixmap p = grab();

  for (const auto &i : qAsConst(itemsToHide)) {
    if (i)
      i->show();
  }

  return p;
};

void CachePlotView::hideSeriesMarker(QLineSeries *series) {
  auto marker =
      std::find_if(m_markers.begin(), m_markers.end(),
                   [=](const auto &mob) { return mob->series == series; });
  Q_ASSERT(marker != m_markers.end());
  marker->get()->clear();
  m_markers.erase(marker);
  resizeObjects(scene()->sceneRect().size());
}

void CachePlotView::showSeriesMarker(QLineSeries *series, const QString &name) {
  m_markers.emplace_back(
      std::make_unique<CachePlotMarker>(this, m_chart, series, name));
  m_markers.rbegin()->get()->marker->setVisible(m_mouseInView);
  resizeObjects(scene()->sceneRect().size());
}

void CachePlotView::setPlot(QChart *chart) {
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

  for (auto *series : chart->series()) {
    if (auto *lineSeries = dynamic_cast<QLineSeries *>(series)) {
      connect(lineSeries, &QLineSeries::clicked, this,
              &CachePlotView::keepCallout);
      connect(lineSeries, &QLineSeries::hovered, this, &CachePlotView::tooltip);
    }
  }

  resizeObjects(m_chart->size());
}

void CachePlotView::resizeObjects(const QSizeF &size) {
  scene()->setSceneRect(QRectF(QPoint(0, 0), size));
  m_chart->resize(size);
  for (auto *item : scene()->items()) {
    if (auto *callout = dynamic_cast<Callout *>(item)) {
      callout->updateGeometry();
    }
  }

  for (unsigned i = 0; i < m_markers.size(); ++i) {
    auto &marker = m_markers.at(i);
    marker->marker->updateLines();
  }
}

void CachePlotView::resizeEvent(QResizeEvent *event) {
  if (scene() && m_chart) {
    resizeObjects(event->size());
  }
  QGraphicsView::resizeEvent(event);
}

void CachePlotView::mouseMoveEvent(QMouseEvent *event) {
  m_hoverPos = event->pos();
  constrainInPlotRange(m_hoverPos, m_chart);

  if (m_markers.size() > 0) {
    for (const auto &marker : m_markers) {
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

void CachePlotView::deleteCallout(Callout *callout) {
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
    m_tooltip->setText(
        QString("Cycle: %1 \nValue: %2 ").arg(point.x()).arg(point.y()));
    m_tooltip->setAnchor(point);
    m_tooltip->setZValue(11);
    m_tooltip->updateGeometry();
    m_tooltip->show();
  } else {
    m_tooltip->hide();
  }
  m_tooltipLock.unlock();
}
} // namespace Ripes
