#include "cacheplotwidget.h"
#include "ui_cacheplotwidget.h"

#include <QClipboard>
#include <QFileDialog>
#include <QPushButton>
#include <QToolBar>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "enumcombobox.h"
#include "processorhandler.h"

#include "limits.h"

namespace {
/**
 * @brief stepifySeries
 * Adds additional points to a QLineSeries, effectively transforming it into a step plot to avoid the default point
 * interpolation of a QLineSeries.
 * We create a new pointsVector, preallocate the known final size and finally exchange the points. This is a lot faster
 * than individually inseting points into the series, which each will trigger events within the QLineSeries.
 */
void stepifySeries(QLineSeries& series) {
    if (series.count() == 0)
        return;

    QVector<QPointF> points;
    points.reserve(series.count() * 2);

    points << series.at(0);  // No stepping is done for the first point
    for (int i = 1; i < series.count(); i++) {
        const QPointF& stepFrom = series.at(i - 1);
        const QPointF& stepTo = series.at(i);
        QPointF interPoint = stepFrom;
        interPoint.setX(stepTo.x());
        points << interPoint << stepTo;
    }

    series.replace(points);
}

inline QPointF stepPoint(const QPointF& p1, const QPointF& p2) {
    return QPointF(p2.x(), p1.y());
}

/**
 * @brief finishSeries
 * Adds an additional point at x value @p max with an equal value of the last value in the series.
 */
void finishSeries(QLineSeries& series, const unsigned max) {
    if (series.count() == 0) {
        return;
    }

    const QPointF lastPoint = series.at(series.count() - 1);
    if (lastPoint.toPoint().x() != static_cast<long>(max)) {
        series.append(max, lastPoint.y());
    }
}

}  // namespace

namespace Ripes {

CachePlotWidget::CachePlotWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CachePlotWidget) {
    m_ui->setupUi(this);
    setWindowTitle("Cache Access Statistics");

    m_toolbar = new QToolBar(this);
    // m_ui->toolbarLayout->addWidget(m_toolbar);
    setupToolbar();

    setupEnumCombobox(m_ui->num, s_cacheVariableStrings);
    setupEnumCombobox(m_ui->den, s_cacheVariableStrings);
    setupEnumCombobox(m_ui->plotType, s_cachePlotTypeStrings);

    setupStackedVariablesList();

    // Set default ratio plot to be hit rate
    setEnumIndex(m_ui->num, Variable::Hits);
    setEnumIndex(m_ui->den, Variable::Accesses);

    connect(m_ui->num, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CachePlotWidget::variablesChanged);
    connect(m_ui->den, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CachePlotWidget::variablesChanged);
    connect(m_ui->stackedVariables, &QListWidget::itemChanged, this, &CachePlotWidget::variablesChanged);

    m_ui->rangeMin->setValue(0);
    m_ui->rangeMax->setValue(ProcessorHandler::getProcessor()->getCycleCount());

    connect(m_ui->rangeMin, QOverload<int>::of(&QSpinBox::valueChanged), this, &CachePlotWidget::rangeChanged);
    connect(m_ui->rangeMax, QOverload<int>::of(&QSpinBox::valueChanged), this, &CachePlotWidget::rangeChanged);

    connect(m_ui->plotType, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &CachePlotWidget::plotTypeChanged);

    const QIcon sizeBreakdownIcon = QIcon(":/icons/info.svg");
    m_ui->sizeBreakdownButton->setIcon(sizeBreakdownIcon);
    connect(m_ui->sizeBreakdownButton, &QPushButton::clicked, this, &CachePlotWidget::showSizeBreakdown);
}

void CachePlotWidget::showSizeBreakdown() {
    QString sizeText;

    const auto cacheSize = m_cache->getCacheSize();

    for (const auto& component : cacheSize.components) {
        sizeText += component + "\n";
    }

    sizeText += "\nTotal: " + QString::number(cacheSize.bits) + " Bits";

    QMessageBox::information(this, "Cache Size Breakdown", sizeText);
}

void CachePlotWidget::setCache(std::shared_ptr<CacheSim> cache) {
    m_cache = cache;

    connect(m_cache.get(), &CacheSim::hitrateChanged, this, &CachePlotWidget::updateHitrate);
    connect(m_cache.get(), &CacheSim::configurationChanged,
            [=] { m_ui->size->setText(QString::number(m_cache->getCacheSize().bits)); });

    // Synchronize widget state
    plotTypeChanged();
    variablesChanged();
    rangeChanged();
    updateHitrate();
}

CachePlotWidget::~CachePlotWidget() {
    delete m_ui;
}

void CachePlotWidget::setupStackedVariablesList() {
    for (const auto& iter : s_cacheVariableStrings) {
        auto* stackedVariableItem = new QListWidgetItem(iter.second);
        stackedVariableItem->setData(Qt::UserRole, QVariant::fromValue<Variable>(iter.first));
        stackedVariableItem->setCheckState(Qt::Unchecked);
        m_ui->stackedVariables->addItem(stackedVariableItem);
    }
}

void CachePlotWidget::setupToolbar() {
    const QIcon crosshairIcon = QIcon(":/icons/crosshair.svg");
    m_crosshairAction = new QAction("Enable plot crosshair", this);
    m_crosshairAction->setIcon(crosshairIcon);
    m_crosshairAction->setCheckable(true);
    m_crosshairAction->setChecked(true);
    m_toolbar->addAction(m_crosshairAction);
    connect(m_crosshairAction, &QAction::triggered, m_ui->plotView, &CachePlotView::enableCrosshair);

    m_toolbar->addSeparator();

    const QIcon copyIcon = QIcon(":/icons/documents.svg");
    m_copyDataAction = new QAction("Copy data to clipboard", this);
    m_copyDataAction->setIcon(copyIcon);
    m_toolbar->addAction(m_copyDataAction);
    connect(m_copyDataAction, &QAction::triggered, this, &CachePlotWidget::copyPlotDataToClipboard);

    const QIcon saveIcon = QIcon(":/icons/saveas.svg");
    m_savePlotAction = new QAction("Save plot to file", this);
    m_savePlotAction->setIcon(saveIcon);
    m_toolbar->addAction(m_savePlotAction);
    connect(m_savePlotAction, &QAction::triggered, this, &CachePlotWidget::savePlot);
}

void CachePlotWidget::plotTypeChanged() {
    m_plotType = getEnumValue<PlotType>(m_ui->plotType);

    if (m_plotType == PlotType::Ratio) {
        m_ui->configWidget->setCurrentWidget(m_ui->ratioConfigPage);
    } else if (m_plotType == PlotType::Stacked) {
        m_ui->configWidget->setCurrentWidget(m_ui->stackedConfigPage);
    } else {
        Q_ASSERT(false);
    }

    variablesChanged();
}

void CachePlotWidget::savePlot() {
    const QString filename = QFileDialog::getSaveFileName(this, "Save file", "", "Images (*.png)");
    if (!filename.isEmpty()) {
        const QPixmap p = m_ui->plotView->getPlotPixmap();
        p.save(filename, "PNG");
    }
}

void CachePlotWidget::copyPlotDataToClipboard() const {
    std::vector<Variable> allVariables;
    for (int i = 0; i < N_Variables; i++) {
        allVariables.push_back(static_cast<Variable>(i));
    }
    const auto& allData = gatherData();

    std::map<unsigned /*cycle*/, QStringList> dataStrings;
    QStringList header;

    // Write cycles
    header << "cycle";
    for (const auto& dataPoint : allData.begin()->second) {
        dataStrings[dataPoint.x()] << QString::number(dataPoint.x());
    }

    // Write variables
    for (const auto& variableData : allData) {
        header << s_cacheVariableStrings.at(variableData.first);
        for (const auto& dataPoint : variableData.second) {
            dataStrings[dataPoint.x()] << QString::number(dataPoint.y());
        }
    }

    // Assemble string
    QString outString;
    outString += header.join('\t') + '\n';

    for (const auto& dataString : dataStrings) {
        outString += dataString.second.join('\t') + '\n';
    }

    QApplication::clipboard()->setText(outString);
}

void CachePlotWidget::rangeChanged() {
    if (m_currentPlot) {
        m_currentPlot->axes(Qt::Horizontal).first()->setRange(m_ui->rangeMin->value(), m_ui->rangeMax->value());
    }

    // Update allowed ranges
    const unsigned cycles = ProcessorHandler::getProcessor()->getCycleCount();
    m_ui->rangeMin->setMinimum(0);
    m_ui->rangeMin->setMaximum(m_ui->rangeMax->value());
    m_ui->rangeMax->setMinimum(m_ui->rangeMin->value());
    m_ui->rangeMax->setMaximum(cycles);
}

std::vector<CachePlotWidget::Variable> CachePlotWidget::gatherVariables() const {
    std::vector<Variable> variables;
    if (m_plotType == PlotType::Ratio) {
        const Variable numerator = getEnumValue<Variable>(m_ui->num);
        const Variable denominator = getEnumValue<Variable>(m_ui->den);
        variables = {numerator, denominator};
    } else if (m_plotType == PlotType::Stacked) {
        for (int i = 0; i < m_ui->stackedVariables->count(); ++i) {
            QListWidgetItem* item = m_ui->stackedVariables->item(i);
            if (item->checkState() == Qt::Checked) {
                variables.push_back(qvariant_cast<Variable>(item->data(Qt::UserRole)));
            }
        }
    } else {
        Q_ASSERT(false);
    }
    return variables;
}

void CachePlotWidget::variablesChanged() {
    const auto vars = gatherVariables();
    if (m_plotType == PlotType::Ratio) {
        Q_ASSERT(vars.size() == 2);
        setPlot(createRatioPlot(vars.at(0), vars.at(1)));
    } else if (m_plotType == PlotType::Stacked) {
        setPlot(createStackedPlot(vars));
    } else {
        Q_ASSERT(false);
    }
}

std::map<CachePlotWidget::Variable, QList<QPoint>> CachePlotWidget::gatherData(unsigned fromCycle) const {
    std::map<Variable, QList<QPoint>> cacheData;
    const auto& trace = m_cache->getAccessTrace();
    // Sanity check to ensure that the trace is keyed with a sequence of [0:N-1] cycles
    Q_ASSERT(((trace.size() == 0) || (trace.size() - 1) == trace.rbegin()->first) &&
             "Non-sequential access access trace?");

    for (int i = 0; i < N_Variables; i++) {
        cacheData[static_cast<Variable>(i)].reserve(trace.size());
    }

    // Gather data
    for (unsigned i = fromCycle; i < trace.size(); i++) {
        const auto& entry = trace.at(i);
        cacheData[Variable::Writes].append(QPoint(i, entry.writes));
        cacheData[Variable::Reads].append(QPoint(i, entry.reads));
        cacheData[Variable::Hits].append(QPoint(i, entry.hits));
        cacheData[Variable::Misses].append(QPoint(i, entry.misses));
        cacheData[Variable::Writebacks].append(QPoint(i, entry.writebacks));
        cacheData[Variable::Accesses].append(QPoint(i, entry.hits + entry.misses));
    }

    return cacheData;
}

void CachePlotWidget::updateRatioPlot(QLineSeries* series, const Variable num, const Variable den) {
    const auto newCacheData = gatherData(series->count());
    const int nNewPoints = newCacheData.at(num).size();

    const auto& numerator = newCacheData.at(num);
    const auto& denominator = newCacheData.at(den);

    double maxY = 0;
    double minY = 9999;

    QList<QPointF> newPoints;
    newPoints.reserve(2 * nNewPoints);
    for (int i = 0; i < nNewPoints; i++) {
        const auto& p1 = numerator.at(i);
        const auto& p2 = denominator.at(i);
        Q_ASSERT(p1.x() == p2.x() && "Data inconsistency");
        double ratio = 0;
        if (p2.y() != 0) {
            ratio = static_cast<double>(p1.y()) / p2.y();
            ratio *= 100;
        }
        const QPointF newPoint = QPointF(p1.x(), ratio);
        series->append(p1.x(), ratio);
        if (series->count() > 0) {
            newPoints << stepPoint(series->pointsVector().last(), newPoint);
        }
        newPoints << newPoint;
        maxY = ratio > maxY ? ratio : maxY;
        minY = ratio < minY ? ratio : minY;
    }
    series->append(newPoints);

    const unsigned maxX = ProcessorHandler::getProcessor()->getCycleCount();

    // Add space to label to add space between labels and axis
    QValueAxis* axisY = qobject_cast<QValueAxis*>(series->chart()->axes(Qt::Vertical).first());
    QValueAxis* axisX = qobject_cast<QValueAxis*>(series->chart()->axes(Qt::Horizontal).first());
    Q_ASSERT(axisY);
    axisY->setLabelFormat("%.1f  ");
    axisY->setTitleText("%");
    int tickInterval = (maxY - minY) / axisY->tickCount();
    tickInterval = ((tickInterval + 5 - 1) / 5) * 5;  // Round to nearest multiple of 5
    if (tickInterval >= 5) {
        axisY->setTickInterval(tickInterval);
        axisY->setTickType(QValueAxis::TicksDynamic);
        axisY->setLabelFormat("%d  ");
    }
    int axisMaxY = maxY * 1.1;
    if (maxY <= 100 && maxY >= 90) {
        axisMaxY = 100;
    }

    axisY->setRange(minY, axisMaxY);

    axisX->setLabelFormat("%d  ");
    axisX->setTitleText("Cycle");
    axisX->setRange(0, maxX);
}

QChart* CachePlotWidget::createRatioPlot(const Variable num, const Variable den) {
    const auto cacheData = gatherData(0);

    QChart* chart = new QChart();
    QLineSeries* series = new QLineSeries(chart);
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->legend()->hide();
    updateRatioPlot(series, num, den);

    return chart;
}

QChart* CachePlotWidget::createStackedPlot(const std::vector<Variable>& variables) const {
    if (variables.size() == 0) {
        return nullptr;
    }

    const auto cacheData = gatherData();
    const unsigned len = cacheData.at(*variables.begin()).size();
    for (const auto& iter : cacheData) {
        Q_ASSERT(static_cast<long>(len) == iter.second.size());
    }

    QChart* chart = new QChart();
    chart->setTitle("Access type count");
    QFont font;
    font.setPointSize(16);
    chart->setTitleFont(font);

    // The lower series initialized to zero values

    // We create a stacked chart by repeatedly creating line series with y values equal to the variable set's y
    // value + the preceding linesets envelope values.
    std::vector<std::pair<Variable, QLineSeries*>> lineSeries;
    QLineSeries* lowerSeries = nullptr;
    QLineSeries* upperSeries = nullptr;
    const unsigned maxX = ProcessorHandler::getProcessor()->getCycleCount();
    unsigned maxY = 0;
    for (const auto& variableData : cacheData) {
        upperSeries = new QLineSeries(chart);
        for (unsigned i = 0; i < len; i++) {
            const auto& dataPoint = variableData.second.at(i);
            unsigned x = dataPoint.x();
            unsigned y = dataPoint.y();
            if (lowerSeries) {
                // Stack on top of the preceding line
                const auto& lowerPoints = lowerSeries->pointsVector();
                y = lowerPoints[i].y() + dataPoint.y();
            }
            maxY = y > maxY ? y : maxY;
            upperSeries->append(QPoint(x, y));
        }
        lineSeries.push_back({variableData.first, upperSeries});
        lowerSeries = upperSeries;
    }

    // Stepify created lineseries
    std::map<Variable, QVector<QPointF>> debug;
    for (const auto& line : lineSeries) {
        debug[line.first] = line.second->pointsVector();
    }
    for (const auto& line : lineSeries) {
        stepifySeries(*line.second);
        finishSeries(*line.second, maxX);
        debug[line.first] = line.second->pointsVector();
    }

    // Create area series
    lowerSeries = nullptr;
    for (unsigned i = 0; i < lineSeries.size(); i++) {
        upperSeries = lineSeries[i].second;
        QAreaSeries* area = new QAreaSeries(upperSeries, lowerSeries);
        area->setName(s_cacheVariableStrings.at(lineSeries[i].first));
        chart->addSeries(area);
        lowerSeries = upperSeries;
    }

    chart->createDefaultAxes();

    // Add space to label to add space between labels and axis
    QValueAxis* axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    QValueAxis* axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    axisX->setRange(0, maxX);
    axisY->setRange(0, axisY->max());

    Q_ASSERT(axisY);
    axisY->setTitleText("#");
    axisX->setTitleText("Cycle");
    axisX->setLabelFormat("%d  ");

    return chart;
}

void CachePlotWidget::setPlot(QChart* plot) {
    if (plot == nullptr)
        return;

    // The plotView takes ownership of @param plot once the plot is set on the view
    m_currentPlot = plot;
    m_ui->plotView->setPlot(m_currentPlot);
}

void CachePlotWidget::updateHitrate() {
    m_ui->hitrate->setText(QString::number(m_cache->getHitRate(), 'G', 4));
    m_ui->hits->setText(QString::number(m_cache->getHits()));
    m_ui->misses->setText(QString::number(m_cache->getMisses()));
    m_ui->writebacks->setText(QString::number(m_cache->getWritebacks()));
}

}  // namespace Ripes
