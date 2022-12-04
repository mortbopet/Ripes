#include "cacheplotwidget.h"
#include "ui_cacheplotwidget.h"

#include <QCheckBox>
#include <QClipboard>
#include <QFileDialog>
#include <QPushButton>
#include <QToolBar>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <algorithm>

#include "colors.h"
#include "enumcombobox.h"
#include "plotconfiguration.h"
#include "processorhandler.h"
#include "ripessettings.h"

#include "limits.h"

namespace Ripes {

const std::map<CachePlotWidget::Variable, QString> s_cacheVariableStrings{
    {CachePlotWidget::Variable::Writes, "Writes"},
    {CachePlotWidget::Variable::Reads, "Reads"},
    {CachePlotWidget::Variable::Hits, "Hits"},
    {CachePlotWidget::Variable::Misses, "Misses"},
    {CachePlotWidget::Variable::Writebacks, "Writebacks"},
    {CachePlotWidget::Variable::Accesses, "Access count"},
    {CachePlotWidget::Variable::Unary, "1"},
    {CachePlotWidget::Variable::WasHit, "Was hit"},
    {CachePlotWidget::Variable::WasMiss, "Was miss"}};

CachePlotWidget::CachePlotWidget(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::CachePlotWidget) {
  m_ui->setupUi(this);
  m_ui->plotView->setScene(new QGraphicsScene(this));
  setWindowTitle("Cache Access Statistics");

  m_ui->rangeMin->setValue(0);
  m_ui->rangeMax->setValue(ProcessorHandler::getProcessor()->getCycleCount());

  connect(m_ui->rangeMin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          [=] { rangeChanged(RangeChangeSource::Comboboxes); });
  connect(m_ui->rangeMax, QOverload<int>::of(&QSpinBox::valueChanged), this,
          [=] { rangeChanged(RangeChangeSource::Comboboxes); });
  connect(m_ui->rangeSlider, &ctkRangeSlider::valuesChanged, this,
          [=] { rangeChanged(RangeChangeSource::Slider); });

  const QIcon sizeBreakdownIcon = QIcon(":/icons/info.svg");
  m_ui->sizeBreakdownButton->setIcon(sizeBreakdownIcon);
  connect(m_ui->sizeBreakdownButton, &QPushButton::clicked, this,
          &CachePlotWidget::showSizeBreakdown);

  connect(m_ui->maxCyclesButton, &QPushButton::clicked, this, [=] {
    QMessageBox::information(
        this, "Maximum plot cycles reached",
        "The maximum number of plot cycles was reached. Beyond this point, "
        "cache statistics is no longer plotted "
        "for performance reasons.\nIf you wish to increase the # of cycles "
        "plotted, please change the setting:\n   "
        " "
        "\"Edit->Settings->Environment->Max. cache plot cycles\"");
  });

  // Setup plot configs tab widget.
  m_ui->plotConfigs->clear();
  QToolButton *addTabButton = new QToolButton();
  addTabButton->setIcon(QIcon(":/icons/plus.svg"));
  addTabButton->setToolTip("Add new plot");
  m_ui->plotConfigs->addTab(new QLabel(), QString());
  m_ui->plotConfigs->setTabEnabled(0, false);
  m_ui->plotConfigs->tabBar()->setTabButton(0, QTabBar::RightSide,
                                            addTabButton);
  auto addTabF = [&]() {
    auto *newPlotConfig = new PlotConfiguration(this);
    int insertPoint = m_ui->plotConfigs->count() - 1;
    m_plotConfigs.insert(newPlotConfig);
    assert(insertPoint >= 0);
    m_ui->plotConfigs->insertTab(insertPoint, newPlotConfig,
                                 "Plot " + QString::number(plotsCreated));
    plotsCreated++;
    auto deleteButton = new QToolButton();
    deleteButton->setIcon(QIcon(":/icons/delete.svg"));
    deleteButton->setToolTip("Remove plot");

    connect(deleteButton, &QToolButton::clicked, this, [=] {
      // Locate the tab that the plot config is currently located at. We don't
      // store this a priori, since these tabs may change based on the order
      // which users added, then deleted, then added, ..., tabs. Safer to just
      // scan all current tabs.
      int tabIdx = -1;

      for (unsigned i = 0; i < m_ui->plotConfigs->count(); ++i) {
        if (m_ui->plotConfigs->widget(i) == newPlotConfig) {
          tabIdx = i;
          break;
        }
      }

      assert(tabIdx >= 0 && "PlotConfig not found in tab widget!");
      newPlotConfig->prepareDelete();
      m_plotConfigs.erase(newPlotConfig);
      m_ui->plotConfigs->removeTab(tabIdx);
      m_ui->plotConfigs->setCurrentIndex(std::max(0, tabIdx - 1));
    });

    m_ui->plotConfigs->tabBar()->setTabButton(insertPoint, QTabBar::RightSide,
                                              deleteButton);

    auto visibleButton = new QCheckBox();
    deleteButton->setToolTip("Show/hide plot");
    visibleButton->setChecked(true);
    connect(visibleButton, &QCheckBox::clicked, this, [=](bool visible) {
      newPlotConfig->setPlotVisible(visible);
      newPlotConfig->setEnabled(visible);
    });

    m_ui->plotConfigs->tabBar()->setTabButton(insertPoint, QTabBar::LeftSide,
                                              visibleButton);

    m_ui->plotConfigs->setCurrentIndex(insertPoint);

    connect(newPlotConfig, &PlotConfiguration::yRangeChanged, this, [this]() {
      // If yRange of a plot has changed we need to go and recalculate the plot
      // axis ranges.
      updatePlotAxes();
    });
  };
  connect(addTabButton, &QToolButton::clicked, this, addTabF);
}

const QString &CachePlotWidget::variableName(Variable var) {
  return s_cacheVariableStrings.at(var);
}

CachePlotView *CachePlotWidget::getPlotView() { return m_ui->plotView; }

void CachePlotWidget::showSizeBreakdown() {
  QString sizeText;

  const auto cacheSize = m_cache->getCacheSize();

  for (const auto &component : cacheSize.components) {
    sizeText += component + "\n";
  }

  sizeText += "\nTotal: " + QString::number(cacheSize.bits) + " Bits";

  QMessageBox::information(this, "Cache Size Breakdown", sizeText);
}

void CachePlotWidget::setCache(const std::shared_ptr<CacheSim> &cache) {
  m_cache = cache;

  connect(m_cache.get(), &CacheSim::hitrateChanged, this,
          &CachePlotWidget::updateHitrate);
  connect(m_cache.get(), &CacheSim::configurationChanged, [=] {
    m_ui->size->setText(QString::number(m_cache->getCacheSize().bits));
  });
  m_ui->size->setText(QString::number(m_cache->getCacheSize().bits));

  connect(ProcessorHandler::get(), &ProcessorHandler::processorClockedNonRun,
          this, &CachePlotWidget::updatePlot);
  connect(ProcessorHandler::get(), &ProcessorHandler::runFinished, this,
          &CachePlotWidget::updatePlot);
  connect(m_cache.get(), &CacheSim::cacheInvalidated, this,
          &CachePlotWidget::resetPlots);

  m_plot = new QChart();
  m_plot->legend()->show();
  m_ui->plotView->setPlot(m_plot);
  setupPlotActions();

  // Setup axes
  m_xAxis = new QValueAxis(this);
  m_yAxis = new QValueAxis(this);
  m_plot->addAxis(m_xAxis, Qt::AlignBottom);
  m_plot->addAxis(m_yAxis, Qt::AlignLeft);

  variablesChanged();
  rangeChanged(RangeChangeSource::Comboboxes);
  updateHitrate();
  updatePlotWarningButton();
  resetPlots();
}

void CachePlotWidget::updateAllowedRange(const RangeChangeSource src) {
  // Block all range signals while changing ranges
  for (const auto &w : m_rangeWidgets) {
    w->blockSignals(true);
  }

  const bool atMax =
      m_ui->rangeSlider->maximumPosition() == m_ui->rangeSlider->maximum();

  m_ui->rangeMin->setMinimum(0);
  m_ui->rangeMax->setMaximum(m_lastCyclePlotted);
  m_ui->rangeSlider->setMinimum(0);
  m_ui->rangeSlider->setMaximum(m_lastCyclePlotted);

  if (src == RangeChangeSource::Cycles) {
    if (atMax) {
      // Keep the range slider at the new maximum position
      m_ui->rangeSlider->setMaximumPosition(m_lastCyclePlotted);
      m_ui->rangeMax->setValue(m_lastCyclePlotted);
    }

    m_ui->rangeMin->setValue(m_lastCyclePlotted < m_ui->rangeMin->value()
                                 ? m_lastCyclePlotted
                                 : m_ui->rangeMin->value());
    m_ui->rangeSlider->setMinimumPosition(
        m_lastCyclePlotted < m_ui->rangeSlider->minimumPosition()
            ? m_lastCyclePlotted
            : m_ui->rangeSlider->minimumPosition());

  } else if (src == RangeChangeSource::Comboboxes) {
    m_ui->rangeSlider->setMinimumPosition(m_ui->rangeMin->value());
    m_ui->rangeSlider->setMaximumPosition(m_ui->rangeMax->value());
  } else if (src == RangeChangeSource::Slider) {
    m_ui->rangeMax->setMinimum(m_ui->rangeSlider->minimumValue());
    m_ui->rangeMin->setMaximum(m_ui->rangeSlider->maximumValue());
    m_ui->rangeMin->setValue(m_ui->rangeSlider->minimumPosition());
    m_ui->rangeMax->setValue(m_ui->rangeSlider->maximumPosition());
  }

  for (const auto &w : m_rangeWidgets) {
    w->blockSignals(false);
  }
}

void CachePlotWidget::updatePlot() {
  // Signal all plot configs to update
  for (auto &plotConfig : m_plotConfigs)
    plotConfig->updateRatioPlot(m_lastCyclePlotted);

  // Update lastCyclePlotted - todo: this is very inefficient - why do we need
  // to go fetch all of the data just to get a single value out?
  auto accessData = gatherData(m_lastCyclePlotted, {}).at(Variable::Accesses);
  if (accessData.size() > 0) {
    m_lastCyclePlotted = accessData.last().x();
  }

  updateAllowedRange(RangeChangeSource::Cycles);
  updatePlotAxes();
  updatePlotWarningButton();
}

CachePlotWidget::~CachePlotWidget() {
  delete m_ui;
  delete m_plot;
}

void CachePlotWidget::setupPlotActions() {

  const QIcon copyIcon = QIcon(":/icons/documents.svg");
  m_copyDataAction = new QAction("Copy data to clipboard", this);
  m_copyDataAction->setIcon(copyIcon);
  m_ui->copyData->setDefaultAction(m_copyDataAction);
  connect(m_copyDataAction, &QAction::triggered, this,
          &CachePlotWidget::copyPlotDataToClipboard);

  const QIcon saveIcon = QIcon(":/icons/saveas.svg");
  m_savePlotAction = new QAction("Save plot to file", this);
  m_savePlotAction->setIcon(saveIcon);
  m_ui->savePlot->setDefaultAction(m_savePlotAction);
  connect(m_savePlotAction, &QAction::triggered, this,
          &CachePlotWidget::savePlot);
}

void CachePlotWidget::savePlot() {
  const QString filename =
      QFileDialog::getSaveFileName(this, "Save file", "", "Images (*.png)");
  if (!filename.isEmpty()) {
    const QPixmap p = m_ui->plotView->getPlotPixmap();
    p.save(filename, "PNG");
  }
}

void CachePlotWidget::copyPlotDataToClipboard() const {
  const auto &allData = gatherData(0, allVariables());

  std::map<unsigned /*cycle*/, QStringList> dataStrings;
  QStringList header;

  // Write cycles
  header << "cycle";
  for (const auto &dataPoint : allData.begin()->second) {
    dataStrings[dataPoint.x()] << QString::number(dataPoint.x());
  }

  // Write variables
  for (const auto &variableData : allData) {
    header << s_cacheVariableStrings.at(variableData.first);
    for (const auto &dataPoint : variableData.second) {
      dataStrings[dataPoint.x()] << QString::number(dataPoint.y());
    }
  }

  // Assemble string
  QString outString;
  outString += header.join('\t') + '\n';

  for (const auto &dataString : dataStrings) {
    outString += dataString.second.join('\t') + '\n';
  }

  QApplication::clipboard()->setText(outString);
}

void CachePlotWidget::rangeChanged(const RangeChangeSource src) {
  updateAllowedRange(src);
  if (m_xAxis)
    m_xAxis->setRange(m_ui->rangeMin->value(), m_ui->rangeMax->value());
}

void CachePlotWidget::variablesChanged() {
  updateAllowedRange(RangeChangeSource::Cycles);
  updatePlotAxes();
}

std::map<CachePlotWidget::Variable, QList<QPoint>>
CachePlotWidget::gatherData(unsigned fromCycle,
                            std::set<Variable> variables) const {
  // Always gather # accesses.
  variables.insert(Variable::Accesses);

  std::map<Variable, QList<QPoint>> cacheData;
  const auto &trace = m_cache->getAccessTrace();

  for (auto &variable : variables)
    cacheData[variable].reserve(trace.size());

  // Gather data up until the end of the trace or the maximum plotted cycles
  const unsigned maxCycles =
      RipesSettings::value(RIPES_SETTING_CACHE_MAXCYCLES).toInt();
  if (fromCycle > maxCycles) {
    return {};
  }

  for (auto it = trace.upper_bound(fromCycle);
       it != trace.lower_bound(maxCycles); it++) {
    const auto &entry = it->second;
    Q_ASSERT(it->first <= maxCycles);
    for (auto &variable : variables) {
      switch (variable) {
      case Variable::Writes:
        cacheData[Variable::Writes].append(QPoint(it->first, entry.writes));
        break;
      case Variable::Reads:
        cacheData[Variable::Reads].append(QPoint(it->first, entry.reads));
        break;
      case Variable::Hits:
        cacheData[Variable::Hits].append(QPoint(it->first, entry.hits));
        break;
      case Variable::Misses:
        cacheData[Variable::Misses].append(QPoint(it->first, entry.misses));
        break;
      case Variable::Writebacks:
        cacheData[Variable::Writebacks].append(
            QPoint(it->first, entry.writebacks));
        break;
      case Variable::Accesses:
        cacheData[Variable::Accesses].append(
            QPoint(it->first, entry.hits + entry.misses));
        break;
      case Variable::WasHit:
        cacheData[Variable::WasHit].append(
            QPoint(it->first, entry.lastTransaction.isHit));
        break;
      case Variable::WasMiss:
        cacheData[Variable::WasMiss].append(
            QPoint(it->first, !entry.lastTransaction.isHit));
        break;
      case Variable::Unary:
        // Handled by special case during plotting.
        break;
      default:
        assert(false && "unknown cache variable");
      }
    }
  }

  return cacheData;
}

void CachePlotWidget::updatePlotAxes() {

  // Add space to label to add space between labels and axis
  Q_ASSERT(m_xAxis);
  Q_ASSERT(m_yAxis);
  m_yAxis->setTitleText("%");

  // Y range is the global maximum of all currently configured plots y ranges.
  double maxY = DBL_MIN;
  double minY = DBL_MAX;
  for (auto &plotConfig : m_plotConfigs) {
    auto &[plotMinY, plotMaxY] = plotConfig->getYRange();
    if (maxY < plotMaxY)
      maxY = plotMaxY;
    if (minY > plotMinY)
      minY = plotMinY;
  }

  int tickInterval = (maxY - minY) / m_yAxis->tickCount();
  tickInterval =
      ((tickInterval + 5 - 1) / 5) * 5; // Round to nearest multiple of 5
  if (tickInterval >= 5) {
    m_yAxis->setTickInterval(tickInterval);
    m_yAxis->setTickType(QValueAxis::TicksDynamic);
    m_yAxis->setLabelFormat("%d  ");
  }
  int axisMaxY = maxY * 1.1;
  if (maxY <= 100 && maxY >= 90) {
    axisMaxY = 100;
  }

  m_xAxis->setLabelFormat("%d  ");
  m_xAxis->setTitleText("Cycle");

  m_yAxis->setRange(minY, axisMaxY);
  m_xAxis->setRange(m_ui->rangeSlider->minimumPosition(),
                    m_ui->rangeSlider->maximumPosition());
}

void CachePlotWidget::updatePlotWarningButton() {
  m_ui->maxCyclesButton->setVisible(
      m_lastCyclePlotted >=
      RipesSettings::value(RIPES_SETTING_CACHE_MAXCYCLES).toInt());
}

void CachePlotWidget::resetPlots() {
  m_lastCyclePlotted = 0;

  for (auto &plotConfig : m_plotConfigs)
    plotConfig->reset();

  updateAllowedRange(RangeChangeSource::Cycles);
  updatePlotAxes();
  updatePlotWarningButton();
}

void CachePlotWidget::updateHitrate() {
  m_ui->hitrate->setText(QString::number(m_cache->getHitRate(), 'G', 4));
  m_ui->hits->setText(QString::number(m_cache->getHits()));
  m_ui->misses->setText(QString::number(m_cache->getMisses()));
  m_ui->writebacks->setText(QString::number(m_cache->getWritebacks()));
}

} // namespace Ripes
