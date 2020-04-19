#include "cacheplotwidget.h"
#include "ui_cacheplotwidget.h"

#include <QClipboard>
#include <QFileDialog>
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

/**
 * @brief finishSeries
 * Adds an additional point at x value @p max with an equal value of the last value in the series.
 */
void finishSeries(QLineSeries& series, const unsigned max) {
    if (series.count() == 0) {
        return;
    }

    const QPointF lastPoint = series.at(series.count() - 1);
    if (lastPoint.toPoint().x() != max) {
        series.append(max, lastPoint.y());
    }
}

}  // namespace

namespace Ripes {

CachePlotWidget::CachePlotWidget(const CacheSim& sim, QWidget* parent)
    : QDialog(parent), m_ui(new Ui::CachePlotWidget), m_cache(sim) {
    m_ui->setupUi(this);
    setWindowTitle("Cache Access Statistics");

    m_toolbar = new QToolBar(this);
    m_ui->toolbarLayout->addWidget(m_toolbar);
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

    const auto& accessTrace = m_cache.getAccessTrace();
    m_ui->rangeMin->setValue(0);
    m_ui->rangeMax->setValue(ProcessorHandler::get()->getProcessor()->getCycleCount());

    connect(m_ui->rangeMin, QOverload<int>::of(&QSpinBox::valueChanged), this, &CachePlotWidget::rangeChanged);
    connect(m_ui->rangeMax, QOverload<int>::of(&QSpinBox::valueChanged), this, &CachePlotWidget::rangeChanged);

    connect(m_ui->plotType, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &CachePlotWidget::plotTypeChanged);

    // Synchronize widget state
    plotTypeChanged();
    variablesChanged();
    rangeChanged();
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
    const auto& allData = gatherData(allVariables);

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
    const auto& accessTrace = m_cache.getAccessTrace();
    const unsigned cycles = ProcessorHandler::get()->getProcessor()->getCycleCount();
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
        setPlot(createRatioPlot(vars[0], vars[1]));
    } else if (m_plotType == PlotType::Stacked) {
        setPlot(createStackedPlot(vars));
    } else {
        Q_ASSERT(false);
    }
}

std::map<CachePlotWidget::Variable, QList<QPoint>>
CachePlotWidget::gatherData(const std::vector<Variable>& types) const {
    const auto& trace = m_cache.getAccessTrace();

    std::map<Variable, QList<QPoint>> data;

    // Transform variable vector to set (avoid duplicates)
    std::set<Variable> varSet;
    for (const auto& type : types) {
        varSet.insert(type);
    }

    for (const auto& type : types) {
        // Initialize all data types
        data[type];
    }

    // Gather data
    for (const auto& entry : trace) {
        if (varSet.count(Variable::Writes)) {
            data[Variable::Writes].append(QPoint(entry.first, entry.second.writes));
        }
        if (varSet.count(Variable::Reads)) {
            data[Variable::Reads].append(QPoint(entry.first, entry.second.reads));
        }
        if (varSet.count(Variable::Hits)) {
            data[Variable::Hits].append(QPoint(entry.first, entry.second.hits));
        }
        if (varSet.count(Variable::Misses)) {
            data[Variable::Misses].append(QPoint(entry.first, entry.second.misses));
        }
        if (varSet.count(Variable::Writebacks)) {
            data[Variable::Writebacks].append(QPoint(entry.first, entry.second.writebacks));
        }
        if (varSet.count(Variable::Accesses)) {
            data[Variable::Accesses].append(QPoint(entry.first, entry.second.hits + entry.second.misses));
        }
    }

    return data;
}

QChart* CachePlotWidget::createRatioPlot(const Variable num, const Variable den) const {
    const auto data = gatherData({num, den});

    const QList<QPoint>& numerator = data.at(num);
    const QList<QPoint>& denominator = data.at(den);

    Q_ASSERT(numerator.size() == denominator.size());

    const unsigned points = numerator.size();

    QChart* chart = new QChart();
    chart->setTitle(s_cacheVariableStrings.at(num) + "/" + s_cacheVariableStrings.at(den));
    QFont font;
    font.setPointSize(16);
    chart->setTitleFont(font);

    QLineSeries* series = new QLineSeries(chart);
    double maxY = 0;
    for (int i = 0; i < points; i++) {
        const auto& p1 = numerator[i];
        const auto& p2 = denominator[i];
        Q_ASSERT(p1.x() == p2.x() && "Data inconsistency");
        double ratio = 0;
        if (p2.y() != 0) {
            ratio = static_cast<double>(p1.y()) / p2.y();
            ratio *= 100;
        }
        series->append(p1.x(), ratio);
        maxY = ratio > maxY ? ratio : maxY;
    }
    const unsigned maxX = ProcessorHandler::get()->getProcessor()->getCycleCount();

    stepifySeries(*series);
    finishSeries(*series, maxX);

    chart->addSeries(series);

    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setRange(0, maxX);
    chart->axes(Qt::Vertical).first()->setRange(0, maxY * 1.1);

    chart->legend()->hide();

    // Add space to label to add space between labels and axis
    QValueAxis* axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    QValueAxis* axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    Q_ASSERT(axisY);
    axisY->setLabelFormat("%.1f  ");
    axisY->setTitleText("%");

    axisX->setLabelFormat("%d  ");
    axisX->setTitleText("Cycle");

    //![4]

    return chart;
}

QChart* CachePlotWidget::createStackedPlot(const std::vector<Variable>& variables) const {
    if (variables.size() == 0) {
        return nullptr;
    }

    const auto data = gatherData(variables);
    const unsigned len = data.at(*variables.begin()).size();
    for (const auto& iter : data) {
        Q_ASSERT(len == iter.second.size());
    }

    QChart* chart = new QChart();
    chart->setTitle("Access type count");
    QFont font;
    font.setPointSize(16);
    chart->setTitleFont(font);

    // The lower series initialized to zero values

    // We create a stacked chart by repeatedly creating line series with y values equal to the variable set's y value +
    // the preceding linesets envelope values.
    std::vector<std::pair<Variable, QLineSeries*>> lineSeries;
    QLineSeries* lowerSeries = nullptr;
    QLineSeries* upperSeries = nullptr;
    const unsigned maxX = ProcessorHandler::get()->getProcessor()->getCycleCount();
    unsigned maxY = 0;
    for (const auto& variableData : data) {
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
    for (int i = 0; i < lineSeries.size(); i++) {
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

    return chart;
}

void CachePlotWidget::setPlot(QChart* plot) {
    if (plot == nullptr)
        return;

    // The plotView takes ownership of @param plot once the plot is set on the view
    m_currentPlot = plot;
    m_ui->plotView->setPlot(m_currentPlot);
}

}  // namespace Ripes
