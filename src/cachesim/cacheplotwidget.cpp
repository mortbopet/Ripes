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

#include <algorithm>

#include "enumcombobox.h"
#include "processorhandler.h"

#include "limits.h"

namespace {

inline QPointF stepPoint(const QPointF& p1, const QPointF& p2) {
    return QPointF(p2.x(), p1.y());
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

    // Set default ratio plot to be hit rate
    setEnumIndex(m_ui->num, Variable::Hits);
    setEnumIndex(m_ui->den, Variable::Accesses);

    connect(m_ui->num, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CachePlotWidget::variablesChanged);
    connect(m_ui->den, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CachePlotWidget::variablesChanged);

    m_ui->rangeMin->setValue(0);
    m_ui->rangeMax->setValue(ProcessorHandler::getProcessor()->getCycleCount());

    connect(m_ui->rangeMin, QOverload<int>::of(&QSpinBox::valueChanged), this, &CachePlotWidget::rangeChanged);
    connect(m_ui->rangeMax, QOverload<int>::of(&QSpinBox::valueChanged), this, &CachePlotWidget::rangeChanged);

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
    connect(ProcessorHandler::get(), &ProcessorHandler::processorClockedNonRun, this,
            &CachePlotWidget::updateRatioPlot);
    connect(ProcessorHandler::get(), &ProcessorHandler::runFinished, this, &CachePlotWidget::updateRatioPlot);
    connect(m_cache.get(), &CacheSim::cacheInvalidated, this, &CachePlotWidget::resetRatioPlot);

    m_plot = new QChart();
    m_series = new QLineSeries(m_plot);
    m_plot->addSeries(m_series);
    m_plot->createDefaultAxes();
    m_plot->legend()->hide();
    m_ui->plotView->setPlot(m_plot);

    variablesChanged();
    rangeChanged();
    updateHitrate();
}

CachePlotWidget::~CachePlotWidget() {
    delete m_ui;
    delete m_plot;
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
    if (m_plot) {
        m_plot->axes(Qt::Horizontal).first()->setRange(m_ui->rangeMin->value(), m_ui->rangeMax->value());
    }

    // Update allowed ranges
    const unsigned cycles = ProcessorHandler::getProcessor()->getCycleCount();
    m_ui->rangeMin->setMinimum(0);
    m_ui->rangeMin->setMaximum(m_ui->rangeMax->value());
    m_ui->rangeMax->setMinimum(m_ui->rangeMin->value());
    m_ui->rangeMax->setMaximum(cycles);
}

void CachePlotWidget::variablesChanged() {
    m_numerator = getEnumValue<Variable>(m_ui->num);
    m_denominator = getEnumValue<Variable>(m_ui->den);
    resetRatioPlot();
}

std::map<CachePlotWidget::Variable, QList<QPoint>> CachePlotWidget::gatherData(unsigned fromCycle) const {
    std::map<Variable, QList<QPoint>> cacheData;
    const auto& trace = m_cache->getAccessTrace();

    for (int i = 0; i < N_Variables; i++) {
        cacheData[static_cast<Variable>(i)].reserve(trace.size());
    }

    // Gather data

    for (auto it = trace.lower_bound(fromCycle); it != trace.end(); it++) {
        const auto& entry = it->second;
        cacheData[Variable::Writes].append(QPoint(it->first, entry.writes));
        cacheData[Variable::Reads].append(QPoint(it->first, entry.reads));
        cacheData[Variable::Hits].append(QPoint(it->first, entry.hits));
        cacheData[Variable::Misses].append(QPoint(it->first, entry.misses));
        cacheData[Variable::Writebacks].append(QPoint(it->first, entry.writebacks));
        cacheData[Variable::Accesses].append(QPoint(it->first, entry.hits + entry.misses));
    }

    return cacheData;
}

void resample(QLineSeries* series, unsigned target, double& step) {
    QVector<QPointF> newPoints;
    const auto& oldPoints = series->pointsVector();
    step = (oldPoints.last().x() / static_cast<double>(target)) * 2;  // *2 to account for steps
    newPoints.reserve(target);
    for (int i = 0; i < oldPoints.size(); i += step) {
        newPoints << oldPoints.at(i);
    }

#if !defined(QT_NO_DEBUG)
    // sanity check
    QPointF plast = QPointF(0, 0);
    bool first = true;
    for (const auto& p : newPoints) {
        if (!first) {
            Q_ASSERT(p.x() - plast.x() < step * 1.1);
            first = false;
        }
        plast = p;
    }
#endif

    series->replace(newPoints);
}

void CachePlotWidget::updateRatioPlot() {
    const auto newCacheData = gatherData(m_lastCyclePlotted);
    const int nNewPoints = newCacheData.at(m_numerator).size();
    if (nNewPoints == 0) {
        return;
    }

    const auto& numerator = newCacheData.at(m_numerator);
    const auto& denominator = newCacheData.at(m_denominator);
    m_lastCyclePlotted = numerator.last().x();

    QList<QPointF> newPoints;
    newPoints.reserve(2 * nNewPoints);
    QPointF lastPoint;
    if (m_series->pointsVector().size() > 0) {
        lastPoint = m_series->pointsVector().last();
    } else {
        lastPoint = QPointF(-1, 0);
    }
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
        if (lastPoint.x() >= 0) {
            if (newPoint.x() - lastPoint.x() < m_xStep) {
                // Skip point; irrelevant at the current sampling level
                continue;
            }
            newPoints << stepPoint(lastPoint, newPoint);
        }
        newPoints << newPoint;
        m_maxY = ratio > m_maxY ? ratio : m_maxY;
        m_minY = ratio < m_minY ? ratio : m_minY;
        lastPoint = newPoint;
    }

    if (newPoints.size() == 0) {
        return;
    }
    // Have to remove series due to append(QList(...)) calling redraw _for each_ point in the list
    m_plot->removeSeries(m_series);
    m_series->append(newPoints);

    // Determine whether to resample; *2 the allowed points to account for the addition of step points.
    const int maxPoints = m_ui->plotView->width() * 2;
    if (m_series->pointsVector().size() >= maxPoints) {
        resample(m_series, maxPoints / 2, m_xStep);
    }

    m_plot->addSeries(m_series);
    m_plot->createDefaultAxes();

    const unsigned maxX = ProcessorHandler::getProcessor()->getCycleCount();

    // Add space to label to add space between labels and axis
    QValueAxis* axisY = qobject_cast<QValueAxis*>(m_series->chart()->axes(Qt::Vertical).first());
    QValueAxis* axisX = qobject_cast<QValueAxis*>(m_series->chart()->axes(Qt::Horizontal).first());
    Q_ASSERT(axisY);
    axisY->setLabelFormat("%.1f  ");
    axisY->setTitleText("%");
    int tickInterval = (m_maxY - m_minY) / axisY->tickCount();
    tickInterval = ((tickInterval + 5 - 1) / 5) * 5;  // Round to nearest multiple of 5
    if (tickInterval >= 5) {
        axisY->setTickInterval(tickInterval);
        axisY->setTickType(QValueAxis::TicksDynamic);
        axisY->setLabelFormat("%d  ");
    }
    int axisMaxY = m_maxY * 1.1;
    if (m_maxY <= 100 && m_maxY >= 90) {
        axisMaxY = 100;
    }

    axisY->setRange(m_minY, axisMaxY);

    axisX->setLabelFormat("%d  ");
    axisX->setTitleText("Cycle");
    axisX->setRange(0, maxX);
}

void CachePlotWidget::resetRatioPlot() {
    m_maxY = -DBL_MAX;
    m_minY = DBL_MAX;
    m_series->clear();
    m_lastCyclePlotted = 0;
    m_xStep = 1;

    updateRatioPlot();
}

void CachePlotWidget::updateHitrate() {
    m_ui->hitrate->setText(QString::number(m_cache->getHitRate(), 'G', 4));
    m_ui->hits->setText(QString::number(m_cache->getHits()));
    m_ui->misses->setText(QString::number(m_cache->getMisses()));
    m_ui->writebacks->setText(QString::number(m_cache->getWritebacks()));
}

}  // namespace Ripes
