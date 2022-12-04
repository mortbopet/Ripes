#pragma once
#include <QAction>
#include <QWidget>

#include "cacheplotwidget.h"

namespace Ripes {

namespace Ui {
class PlotConfiguration;
}

class PlotConfiguration : public QWidget {
  Q_OBJECT

public:
  explicit PlotConfiguration(CachePlotWidget *parent = nullptr);
  ~PlotConfiguration();

  void reset();

  std::pair<double, double> getYRange() const { return {m_minY, m_maxY}; }
  void updateRatioPlot(unsigned lastCyclePlotted);
  QString getName();
  void prepareDelete();
  void setPlotVisible(bool visible);

signals:
  void yRangeChanged();

private slots:
  void variablesChanged();

private:
  enum class PlotType { total, mavg };
  void setColor(QColor color, PlotType type);
  void addSeriesToPlot();
  void resetRatioPlot();

  double m_maxY = -DBL_MAX;
  double m_minY = DBL_MAX;
  double m_xStep = 1.0;

  Ui::PlotConfiguration *m_ui;

  CachePlotWidget::Variable m_numerator;
  CachePlotWidget::Variable m_denominator;

  CachePlotWidget *cpw;

  QLineSeries *m_series = nullptr;
  QColor seriesColor;
  QLineSeries *m_mavgSeries = nullptr;
  QColor mavgColor;
  FixedQueue<double> m_mavgData;

  QAction *m_ratioMarkerAction = nullptr;
  QAction *m_mavgMarkerAction = nullptr;
  bool m_visible = true;

  // Color state for getting semi-unique colors to initialize plots with.
  unsigned nextColorID = 0;
  QColor getNextColor();
};

} // namespace Ripes
