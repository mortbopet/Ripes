#pragma once

#include <QMetaType>
#include <QWidget>
#include <QtCharts/QChartGlobal>

#include "cachesim.h"
#include "float.h"
#include <queue>

QT_FORWARD_DECLARE_CLASS(QToolBar);
QT_FORWARD_DECLARE_CLASS(QAction);

class QChartView;
class QChart;
class QLineSeries;

template <typename T>
class FixedQueue : public std::deque<T> {
public:
  FixedQueue(const unsigned N = 1) : m_n(N) {}
  void push(const T &value) {
    if (this->size() == m_n) {
      this->pop_front();
    }
    std::deque<T>::push_back(value);
  }

private:
  unsigned m_n;
};

namespace Ripes {

namespace Ui {
class CachePlotWidget;
}

class CachePlotWidget : public QWidget {
  Q_OBJECT

  enum class RangeChangeSource { Slider, Comboboxes, Cycles };

public:
  enum Variable {
    Writes = 0,
    Reads,
    Hits,
    Misses,
    WasHit,
    WasMiss,
    Writebacks,
    Accesses,
    N_TraceVars,
    Unary
  };
  explicit CachePlotWidget(QWidget *parent = nullptr);
  void setCache(const std::shared_ptr<CacheSim> &cache);
  ~CachePlotWidget();

public slots:

private slots:
  void variablesChanged();
  void rangeChanged(const CachePlotWidget::RangeChangeSource src);
  void updateHitrate();

private:
  /**
   * @brief gatherData
   * @returns a list of QPoints containing plotable data gathered from the cache
   * simulator, starting from the specified cycle
   */
  std::map<Variable, QList<QPoint>> gatherData(unsigned fromCycle = 0) const;
  void setupPlotActions();
  void showSizeBreakdown();
  void copyPlotDataToClipboard() const;
  void savePlot();
  void updateRatioPlot();
  void updatePlotAxes();
  void updateAllowedRange(const RangeChangeSource src);
  void updatePlotWarningButton();

  /**
   * @brief resampleToScreen
   * Resamples @param series to only contain as many points as would be visible
   * on the associated plot view
   */
  void resampleToScreen(QLineSeries *series);

  void resetRatioPlot();
  QChart *m_plot = nullptr;
  QLineSeries *m_series = nullptr;
  double m_maxY = -DBL_MAX;
  double m_minY = DBL_MAX;
  int64_t m_lastCyclePlotted = 0;
  double m_xStep = 1.0;
  static constexpr int s_resamplingRatio = 2;

  QLineSeries *m_mavgSeries = nullptr;
  // N last computations of the change in ratio value
  FixedQueue<double> m_mavgData;

  Ui::CachePlotWidget *m_ui;
  std::shared_ptr<CacheSim> m_cache;

  CachePlotWidget::Variable m_numerator;
  CachePlotWidget::Variable m_denominator;

  QAction *m_copyDataAction = nullptr;
  QAction *m_savePlotAction = nullptr;
  QAction *m_ratioMarkerAction = nullptr;
  QAction *m_mavgMarkerAction = nullptr;

  std::vector<QWidget *> m_rangeWidgets;
};

const static std::map<CachePlotWidget::Variable, QString>
    s_cacheVariableStrings{
        {CachePlotWidget::Variable::Writes, "Writes"},
        {CachePlotWidget::Variable::Reads, "Reads"},
        {CachePlotWidget::Variable::Hits, "Hits"},
        {CachePlotWidget::Variable::Misses, "Misses"},
        {CachePlotWidget::Variable::Writebacks, "Writebacks"},
        {CachePlotWidget::Variable::Accesses, "Access count"},
        {CachePlotWidget::Variable::Unary, "1"},
        {CachePlotWidget::Variable::WasHit, "Was hit"},
        {CachePlotWidget::Variable::WasMiss, "Was miss"}};

} // namespace Ripes

Q_DECLARE_METATYPE(Ripes::CachePlotWidget::Variable);
