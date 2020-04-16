#include "cacheplotwidget.h"
#include "ui_cacheplotwidget.h"

#include <QToolBar>
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
        stackedVariableItem->setData(Qt::UserRole, iter.second);
        stackedVariableItem->setCheckState(Qt::Unchecked);
        m_ui->stackedVariables->addItem(stackedVariableItem);
    }
}

void CachePlotWidget::setupToolbar() {
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
        setPlot(numerator, denominator);
    }
}

QMap<CachePlotWidget::Variable, QList<QPoint>> CachePlotWidget::gatherData(const std::set<Variable> types) const {
    const auto& trace = m_cache.getAccessTrace();

    QMap<CachePlotWidget::Variable, QList<QPoint>> data;

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

    const QList<QPoint>& numerator = data[num];
    const QList<QPoint>& denominator = data[den];

    Q_ASSERT(numerator.size() == denominator.size());

    const unsigned points = numerator.size();

    QChart* chart = new QChart();
    chart->setTitle(s_cacheVariableStrings[num] + "/" + s_cacheVariableStrings[den]);

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

    chart->legend()->setEnabled(false);

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

void CachePlotWidget::setPlot(const Variable num, const Variable den) {
    const auto& accessTrace = m_cache.getAccessTrace();

    auto* oldChart = m_ui->plotView->chart();
    m_currentPlot = createRatioPlot(num, den);
    m_ui->plotView->setChart(m_currentPlot);
    if (oldChart) {
        delete oldChart;
    }
}

}  // namespace Ripes
