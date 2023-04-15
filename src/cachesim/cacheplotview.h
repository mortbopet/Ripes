#pragma once

#include <QChart>
#include <QMutex>
#include <QtCharts/QChartGlobal>
#include <QtWidgets/QGraphicsView>

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QMouseEvent;
class QResizeEvent;
QT_END_NAMESPACE

class QChart;
class QLineSeries;

class Callout;

namespace Ripes {
class ChartLineMarker;

class CachePlotMarker : public QObject {
  Q_OBJECT

public:
  ChartLineMarker *marker = nullptr;
  QLineSeries *series = nullptr;
  QString name;
  CachePlotMarker(QObject *parent, QChart *chart, QLineSeries *series,
                  const QString &name);
  void updateCoordinateValues(const QPointF &pos, bool showValue,
                              bool showCycles);
  void clear();
};

class CachePlotView : public QGraphicsView {
  Q_OBJECT

public:
  CachePlotView(QWidget *parent = nullptr);

  void setPlot(QChart *chart);

  /**
   * @brief getPlotPixmap
   * @returns a pixmap containing only the plot (ie. no coordinate values nor
   * marker)
   */
  QPixmap getPlotPixmap();

protected:
  void resizeEvent(QResizeEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;

public slots:
  void keepCallout();
  void deleteCallout(Callout *callout);
  void tooltip(QPointF point, bool state);
  void hideSeriesMarker(QLineSeries *series);
  void showSeriesMarker(QLineSeries *series, const QString &name);

private:
  void resizeObjects(const QSizeF &size);

  QPointF m_hoverPos;

  std::vector<std::unique_ptr<CachePlotMarker>> m_markers;

  QChart *m_chart = nullptr;
  Callout *m_tooltip = nullptr;
  QMutex m_tooltipLock;

  bool m_mouseInView = false;
  bool m_crosshairEnabled = true;
};

} // namespace Ripes
