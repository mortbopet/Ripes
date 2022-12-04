#include "plotconfiguration.h"
#include "ui_plotconfiguration.h"

#include "cacheplotview.h"
#include "enumcombobox.h"
#include "ripessettings.h".

#include <QColorDialog>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "colors.h"

using namespace Ripes;

static void resample(QLineSeries *series, unsigned target, double &step) {
  QVector<QPointF> newPoints;
  const auto &oldPoints = series->pointsVector();
  step = (oldPoints.last().x() / static_cast<double>(target)) *
         2; // *2 to account for steps
  newPoints.reserve(target);
  for (int i = 0; i < oldPoints.size(); i += step) {
    newPoints << oldPoints.at(i);
  }

#if !defined(QT_NO_DEBUG)
  // sanity check
  QPointF plast = QPointF(0, 0);
  bool first = true;
  for (const auto &p : qAsConst(newPoints)) {
    if (!first) {
      Q_ASSERT(p.x() - plast.x() < step * 1.1);
      first = false;
    }
    plast = p;
  }
#endif

  series->replace(newPoints);
}

QColor PlotConfiguration::getNextColor() {
  // Based on Paul Tol's vibrant color schemes.
  // https://personal.sron.nl/~pault/data/colourschemes.pdf
  static QStringList colorNames{"#0077BB", "#33BBEE", "#009988", "#EE7733",
                                "#CC3311", "#EE3377", "#BBBBBB", "#000000"};
  if (nextColorID >= colorNames.size()) {
    // Wraparound
    nextColorID = 0;
  }
  auto color = QColor(colorNames[nextColorID]);
  nextColorID++;
  return color;
}

static QPointF stepPoint(const QPointF &p1, const QPointF &p2) {
  return QPointF(p2.x(), p1.y());
}

void PlotConfiguration::setColor(QColor color,
                                 PlotConfiguration::PlotType type) {
  auto &button =
      type == PlotType::mavg ? m_ui->mavgColorButton : m_ui->ratioColorButton;
  auto &series = type == PlotType::mavg ? m_mavgSeries : m_series;

  auto size = button->size();
  QPixmap pm(size);
  pm.fill(color);
  button->setIcon(pm);

  auto pen = series->pen(); // Inherit default pen state
  pen.setColor(color);
  series->setPen(pen);
}

PlotConfiguration::PlotConfiguration(CachePlotWidget *parent)
    : QWidget(parent), m_ui(new Ui::PlotConfiguration), cpw(parent) {
  m_ui->setupUi(this);

  setupEnumCombobox(m_ui->num, s_cacheVariableStrings);
  setupEnumCombobox(m_ui->den, s_cacheVariableStrings);

  // Set default ratio plot to be cummulative hitrate over time.
  setEnumIndex(m_ui->num, CachePlotWidget::Variable::Hits);
  setEnumIndex(m_ui->den, CachePlotWidget::Variable::Accesses);

  connect(m_ui->num, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &PlotConfiguration::variablesChanged);
  connect(m_ui->den, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &PlotConfiguration::variablesChanged);
  connect(m_ui->showMAvg, &QCheckBox::toggled, this,
          &PlotConfiguration::variablesChanged);
  connect(m_ui->showRatio, &QCheckBox::toggled, this,
          &PlotConfiguration::variablesChanged);

  connect(m_ui->windowCycles, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &PlotConfiguration::variablesChanged);
  connect(m_ui->showMAvg, &QCheckBox::toggled, m_ui->windowCycles,
          &QWidget::setEnabled);

  connect(m_ui->mavgColorButton, &QToolButton::clicked, this, [=]() {
    mavgColor = QColorDialog::getColor(mavgColor);
    setColor(mavgColor, PlotType::mavg);
  });
  connect(m_ui->ratioColorButton, &QToolButton::clicked, this, [=]() {
    seriesColor = QColorDialog::getColor(seriesColor);
    setColor(seriesColor, PlotType::total);
  });

  m_series = new QLineSeries(cpw->m_plot);
  cpw->getPlotView()->addSeries(m_series, "total");
  seriesColor = getNextColor();
  setColor(seriesColor, PlotType::total);

  m_mavgSeries = new QLineSeries(cpw->m_plot);
  cpw->getPlotView()->addSeries(m_mavgSeries, "ratio");
  mavgColor = getNextColor();
  setColor(mavgColor, PlotType::mavg);

  auto showMarkerFunctor = [=](QLineSeries *series) {
    return [=](bool visible) {
      if (visible) {
        cpw->getPlotView()->showSeriesMarker(series);
      } else {
        cpw->getPlotView()->hideSeriesMarker(series);
      }
    };
  };

  // Setup ratio marker.
  m_ratioMarkerAction = new QAction("Enable ratio crosshair", this);
  m_ratioMarkerAction->setIcon(QIcon(":/icons/crosshair.svg"));
  m_ratioMarkerAction->setCheckable(true);
  m_ui->ratioCursor->setDefaultAction(m_ratioMarkerAction);
  connect(m_ratioMarkerAction, &QAction::toggled, showMarkerFunctor(m_series));
  m_ratioMarkerAction->setChecked(true);
  connect(m_ui->showRatio, &QCheckBox::toggled, m_ratioMarkerAction,
          [=](bool enabled) {
            m_ratioMarkerAction->setChecked(enabled);
            m_ratioMarkerAction->setEnabled(enabled);
          });

  // Setup moving average marker.
  m_mavgMarkerAction = new QAction("Enable moving average crosshair", this);
  m_mavgMarkerAction->setIcon(QIcon(":/icons/crosshair.svg"));
  m_mavgMarkerAction->setCheckable(true);
  m_ui->mavgCursor->setDefaultAction(m_mavgMarkerAction);
  connect(m_mavgMarkerAction, &QAction::toggled,
          showMarkerFunctor(m_mavgSeries));
  m_mavgMarkerAction->setChecked(true);
  connect(m_ui->showMAvg, &QCheckBox::toggled, m_mavgMarkerAction,
          [=](bool enabled) {
            m_mavgMarkerAction->setChecked(enabled);
            m_mavgMarkerAction->setEnabled(enabled);
          });

  variablesChanged();
}

void PlotConfiguration::prepareDelete() {
  auto *plot = cpw->getPlotView();
  plot->removeSeries(m_series);
  plot->removeSeries(m_mavgSeries);
}

PlotConfiguration::~PlotConfiguration() { delete m_ui; }

QString PlotConfiguration::getName() {
  return cpw->variableName(m_numerator) + "/" +
         cpw->variableName(m_denominator);
}

void PlotConfiguration::variablesChanged() {
  m_numerator = getEnumValue<CachePlotWidget::Variable>(m_ui->num);
  m_denominator = getEnumValue<CachePlotWidget::Variable>(m_ui->den);

  // Define new name for the plot.
  auto name = getName();
  cpw->getPlotView()->setSeriesName(m_series, name);
  cpw->getPlotView()->setSeriesName(m_mavgSeries, name + " (m.avg)");

  reset();
  // Plot from cycle 0.
  updateRatioPlot(0);

  // Emit yRangeChanged so parent plotter can rescale accordingly.
  emit yRangeChanged();
}

void PlotConfiguration::reset() {
  m_maxY = -DBL_MAX;
  m_minY = DBL_MAX;
  m_xStep = 1.0;

  m_series->clear();
  m_mavgSeries->clear();

  if (m_ui->showMAvg->isChecked()) {
    m_mavgData = FixedQueue<double>(m_ui->windowCycles->value());
  }

  setPlotVisible(m_visible);
}

void PlotConfiguration::setPlotVisible(bool visible) {
  m_visible = visible;
  m_mavgSeries->setVisible(m_visible ? m_ui->showMAvg->isChecked() : false);
  m_series->setVisible(m_visible ? m_ui->showRatio->isChecked() : false);
}

void PlotConfiguration::updateRatioPlot(unsigned lastCyclePlotted) {
  const auto newCacheData =
      cpw->gatherData(lastCyclePlotted, {m_numerator, m_denominator});
  const int nNewPoints = newCacheData.at(CachePlotWidget::Accesses).size();
  if (nNewPoints == 0) {
    return;
  }

  // convenience function to move a series on/off the chart based on the number
  // of points to be added. We have to remove series due to append(QList(...))
  // calling redraw _for each_ point in the list. Not removing the series for
  // low nNewPoints avoids a flicker in plot labels. Everything is a balance...
  const auto plotMover = [=](QLineSeries *series, bool visible) {
    if (nNewPoints > 2) {
      if (visible) {
        cpw->getPlotView()->addSeries(series, getName());
      } else {
        cpw->getPlotView()->removeSeries(series);
      }
    }
  };

  QList<QPointF> newPoints;
  QList<QPointF> newWindowPoints;
  QPointF lastPoint;
  if (m_series->pointsVector().size() > 0) {
    lastPoint = m_series->pointsVector().constLast();
  } else {
    lastPoint = QPointF(-1, 0);
  }
  for (int i = 0; i < nNewPoints; ++i) {
    // Cummulative plot. For the unary variable, "Accesses" is just used to
    // index into the cache data for accessing the x variable.
    const auto &p1 =
        m_numerator == CachePlotWidget::Unary
            ? QPoint(newCacheData.at(CachePlotWidget::Accesses).at(i).x(), 1)
            : newCacheData.at(m_numerator).at(i);
    const auto &p2 =
        m_denominator == CachePlotWidget::Unary
            ? QPoint(newCacheData.at(CachePlotWidget::Accesses).at(i).x(), 1)
            : newCacheData.at(m_denominator).at(i);
    Q_ASSERT(p1.x() == p2.x() && "Data inconsistency");
    double ratio = 0;
    if (p2.y() != 0) {
      ratio = static_cast<double>(p1.y()) / p2.y();
      ratio *= 100.0;
    }
    bool skipPoint = false;
    const QPointF newPoint = QPointF(p1.x(), ratio);
    if (lastPoint.x() >= 0) {
      if (newPoint.x() - lastPoint.x() < m_xStep) {
        // Skip point; irrelevant at the current sampling level
        skipPoint = true;
      }
    }
    if (!skipPoint) {
      newPoints << stepPoint(lastPoint, newPoint);
      newPoints << newPoint;
      m_maxY = ratio > m_maxY ? ratio : m_maxY;
      m_minY = ratio < m_minY ? ratio : m_minY;
      lastPoint = newPoint;
    }

    // Moving average plot
    if (m_ui->showMAvg->isChecked()) {
      m_mavgData.push(newPoint.y());
      const double wAvg =
          std::accumulate(m_mavgData.begin(), m_mavgData.end(), 0.0) /
          m_mavgData.size();
      newWindowPoints << QPointF(p1.x(), wAvg);
    }
  }

  if (newPoints.size() == 0 && newWindowPoints.size() == 0) {
    return;
  }

  plotMover(m_series, false);
  m_series->append(newPoints);
  if (m_ui->showMAvg->isChecked()) {
    plotMover(m_mavgSeries, false);
    m_mavgSeries->append(newWindowPoints);
  }

  // Determine whether to resample;
  // *2 the allowed points to account for the addition of step points.
  // *2 to account for the fact that the setting specifies the minimum # of
  // points, and we resample at a 2x ratio
  const int maxPoints =
      RipesSettings::value(RIPES_SETTING_CACHE_MAXPOINTS).toInt() * 2 * 2;
  if (m_series->pointsVector().size() >= maxPoints) {
    resample(m_series, maxPoints / 2, m_xStep);
  }

  plotMover(m_series, true);
  if (m_ui->showMAvg->isChecked()) {
    plotMover(m_mavgSeries, true);
  }
}
