#include "cacheplotwidget.h"
#include "ui_cacheplotwidget.h"

#include <QToolBar>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "enumcombobox.h"

#include "limits.h"

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
    m_ui->rangeMax->setValue(accessTrace.size());

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
    m_copyDataAction = new QAction("Copy plot data to clipboard", this);
    m_copyDataAction->setIcon(copyIcon);
    m_toolbar->addAction(m_copyDataAction);

    const QIcon saveIcon = QIcon(":/icons/save.svg");
    m_savePlotAction = new QAction("Save plot to file", this);
    m_savePlotAction->setIcon(saveIcon);
    m_toolbar->addAction(m_savePlotAction);
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

void CachePlotWidget::rangeChanged() {
    if (m_currentPlot) {
        m_currentPlot->axes(Qt::Horizontal).first()->setRange(m_ui->rangeMin->value(), m_ui->rangeMax->value());
    }

    // Update allowed ranges
    const auto& accessTrace = m_cache.getAccessTrace();
    m_ui->rangeMin->setMinimum(0);
    m_ui->rangeMin->setMaximum(m_ui->rangeMax->value());
    m_ui->rangeMax->setMinimum(m_ui->rangeMin->value());
    m_ui->rangeMax->setMaximum(accessTrace.size());
}

void CachePlotWidget::variablesChanged() {
    if (m_plotType == PlotType::Ratio) {
        const Variable numerator = getEnumValue<Variable>(m_ui->num);
        const Variable denominator = getEnumValue<Variable>(m_ui->den);
        setPlot(createRatioPlot(numerator, denominator));
    } else if (m_plotType == PlotType::Stacked) {
        std::set<Variable> vars;
        for (int i = 0; i < m_ui->stackedVariables->count(); ++i) {
            QListWidgetItem* item = m_ui->stackedVariables->item(i);
            if (item->checkState() == Qt::Checked) {
                vars.insert(qvariant_cast<Variable>(item->data(Qt::UserRole)));
            }
        }
        setPlot(createStackedPlot(vars));
    } else {
        Q_ASSERT(false);
    }
}

std::map<CachePlotWidget::Variable, QList<QPoint>> CachePlotWidget::gatherData(const std::set<Variable> types) const {
    const auto& trace = m_cache.getAccessTrace();

    std::map<CachePlotWidget::Variable, QList<QPoint>> data;

    for (const auto& type : types) {
        // Initialize all data types
        data[type];
    }

    // Gather data
    for (const auto& entry : trace) {
        if (types.count(Variable::Writes)) {
            data[Variable::Writes].append(QPoint(entry.first, entry.second.writes));
        }
        if (types.count(Variable::Reads)) {
            data[Variable::Reads].append(QPoint(entry.first, entry.second.reads));
        }
        if (types.count(Variable::Hits)) {
            data[Variable::Hits].append(QPoint(entry.first, entry.second.hits));
        }
        if (types.count(Variable::Misses)) {
            data[Variable::Misses].append(QPoint(entry.first, entry.second.misses));
        }
        if (types.count(Variable::Writebacks)) {
            data[Variable::Writebacks].append(QPoint(entry.first, entry.second.writebacks));
        }
        if (types.count(Variable::Accesses)) {
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

    QLineSeries* series = new QLineSeries(chart);
    double maxval = 0;
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
        maxval = ratio > maxval ? ratio : maxval;
    }

    chart->addSeries(series);

    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setRange(0, points);
    chart->axes(Qt::Vertical).first()->setRange(0, maxval * 1.1);

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

QChart* CachePlotWidget::createStackedPlot(const std::set<Variable>& variables) const {
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

    // The lower series initialized to zero values

    // We create a stacked chart by repeatedly creating line series with y values equal to the variable set's y value +
    // the preceding linesets envelope values.
    QLineSeries* lowerSeries = nullptr;
    unsigned maxValue = 0;
    for (const auto& variableData : data) {
        QLineSeries* upperSeries = new QLineSeries(chart);
        for (unsigned i = 0; i < len; i++) {
            const auto& dataPoint = variableData.second.at(i);
            unsigned x = dataPoint.x();
            unsigned y = dataPoint.y();
            if (lowerSeries) {
                // Stack on top of the preceding line
                const auto& lowerPoints = lowerSeries->pointsVector();
                y = lowerPoints[i].y() + dataPoint.y();
            }
            maxValue = y > maxValue ? y : maxValue;
            upperSeries->append(QPoint(x, y));
        }
        QAreaSeries* area = new QAreaSeries(upperSeries, lowerSeries);
        area->setName(s_cacheVariableStrings.at(variableData.first));
        chart->addSeries(area);
        lowerSeries = upperSeries;
    }

    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setRange(0, len);
    chart->axes(Qt::Vertical).first()->setRange(0, maxValue);

    // Add space to label to add space between labels and axis
    QValueAxis* axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    QValueAxis* axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    Q_ASSERT(axisY);
    axisY->setLabelFormat("%d  ");
    axisY->setTitleText("#");

    axisX->setLabelFormat("%d  ");
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
