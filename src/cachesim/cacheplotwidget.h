#pragma once

#include <QDialog>
#include <QMetaType>
#include <QtCharts/QChartGlobal>

#include "cachesim.h"

QT_FORWARD_DECLARE_CLASS(QToolBar);
QT_FORWARD_DECLARE_CLASS(QAction);

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

namespace Ripes {

namespace Ui {
class CachePlotWidget;
}

class CachePlotWidget : public QDialog {
    Q_OBJECT

public:
    enum Variable { Writes = 0, Reads, Hits, Misses, Writebacks, Accesses, N_Variables };
    enum class PlotType { Ratio, Stacked };
    explicit CachePlotWidget(const CacheSim& sim, QWidget* parent = nullptr);
    ~CachePlotWidget();

public slots:

private slots:
    void variablesChanged();
    void rangeChanged();
    void plotTypeChanged();

private:
    /**
     * @brief gatherData
     * @returns a list of QPoints containing plotable data gathered from the cache simulator, as per the specified
     */
    std::map<Variable, QList<QPoint>> gatherData(const std::vector<Variable>& variables) const;
    void setupToolbar();
    void setupStackedVariablesList();
    void setPlot(QChart* plot);
    void copyPlotDataToClipboard() const;
    void savePlot();
    std::vector<CachePlotWidget::Variable> gatherVariables() const;

    QChart* createRatioPlot(const Variable num, const Variable den) const;
    QChart* createStackedPlot(const std::vector<Variable>& variables) const;

    PlotType m_plotType = PlotType::Ratio;
    QChart* m_currentPlot = nullptr;

    Ui::CachePlotWidget* m_ui;
    const CacheSim& m_cache;

    QToolBar* m_toolbar = nullptr;
    QAction* m_copyDataAction = nullptr;
    QAction* m_savePlotAction = nullptr;
    QAction* m_crosshairAction = nullptr;
};

const static std::map<CachePlotWidget::Variable, QString> s_cacheVariableStrings{
    {CachePlotWidget::Variable::Writes, "Writes"},
    {CachePlotWidget::Variable::Reads, "Reads"},
    {CachePlotWidget::Variable::Hits, "Hits"},
    {CachePlotWidget::Variable::Misses, "Misses"},
    {CachePlotWidget::Variable::Writebacks, "Writebacks"},
    {CachePlotWidget::Variable::Accesses, "Total accesses"}};

const static std::map<CachePlotWidget::PlotType, QString> s_cachePlotTypeStrings{
    {CachePlotWidget::PlotType::Ratio, "Ratio"},
    {CachePlotWidget::PlotType::Stacked, "Stacked"}};

}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::CachePlotWidget::Variable);
Q_DECLARE_METATYPE(Ripes::CachePlotWidget::PlotType);
