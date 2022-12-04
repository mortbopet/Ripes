#pragma once

#include <QMetaType>
#include <QWidget>
#include <QtCharts/QChartGlobal>

#include "cachesim.h"
#include "float.h"
#include <queue>

QT_FORWARD_DECLARE_CLASS(QToolBar);
QT_FORWARD_DECLARE_CLASS(QAction);

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
class QLineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

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

class PlotConfiguration;
class CachePlotView;

class CachePlotWidget : public QWidget {
  Q_OBJECT

  friend class PlotConfiguration;

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

  static const QString &variableName(Variable);

  static std::set<Variable> allVariables() {
    return {Writes,  Reads,      Hits,     Misses, WasHit,
            WasMiss, Writebacks, Accesses, Unary};
  }

  explicit CachePlotWidget(QWidget *parent = nullptr);
  void setCache(const std::shared_ptr<CacheSim> &cache);
  ~CachePlotWidget();

  /**
   * @brief gatherData
   * @returns a list of QPoints containing plotable data gathered from the cache
   * simulator, starting from the specified cycle and for the requested cache
   * variables.
   */
  std::map<Variable, QList<QPoint>>
  gatherData(unsigned fromCycle, std::set<Variable> variables) const;

  CachePlotView *getPlotView();

public slots:

private slots:
  void variablesChanged();
  void rangeChanged(const CachePlotWidget::RangeChangeSource src);
  void updateHitrate();

private:
  void setupPlotActions();
  void showSizeBreakdown();
  void copyPlotDataToClipboard() const;
  void savePlot();
  void updatePlot();
  void updatePlotAxes();
  void updateAllowedRange(const RangeChangeSource src);
  void updatePlotWarningButton();

  /**
   * @brief resampleToScreen
   * Resamples @param series to only contain as many points as would be visible
   * on the associated plot view
   */
  void resampleToScreen(QLineSeries *series);

  void resetPlots();
  QChart *m_plot = nullptr;
  QValueAxis *m_xAxis = nullptr;
  QValueAxis *m_yAxis = nullptr;
  int64_t m_lastCyclePlotted = 0;
  static constexpr int s_resamplingRatio = 2;

  // Number of plots created.
  unsigned plotsCreated = 1;

  Ui::CachePlotWidget *m_ui;
  std::shared_ptr<CacheSim> m_cache;

  QAction *m_copyDataAction = nullptr;
  QAction *m_savePlotAction = nullptr;

  std::vector<QWidget *> m_rangeWidgets;

  // Map between a plot config and its current tab index.
  std::set<PlotConfiguration *> m_plotConfigs;
};

extern const std::map<CachePlotWidget::Variable, QString>
    s_cacheVariableStrings;

} // namespace Ripes

Q_DECLARE_METATYPE(Ripes::CachePlotWidget::Variable);
