#include "cacheinspector.h"
#include "ui_cacheinspector.h"

#include "qcustomplot.h"

CacheInspector::CacheInspector(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheInspector) {
    m_ui->setupUi(this);

    // Generate some data
    m_hitData = {{0}, {0}, {0}};
    m_missData = {{0}, {0}, {0}};
    int dataSize = 1000;
    for (int i = 0; i < 3; i++) {
        for (int j = 1; j < dataSize; j++) {
            // create data that increases hits linearly
            int r = (rand() % dataSize);
            if (r * (i + 0.5) > j) {
                m_missData[i].append(m_missData[i].last() + 1);
                m_hitData[i].append(m_hitData[i].last());
            } else {
                m_hitData[i].append(m_hitData[i].last() + 1);
                m_missData[i].append(m_missData[i].last());
            }
        }
    }

    setupCacheRequestPlot();
    setupCacheRequestRatioPlot();
    setupTemporalRatioPlot();

    // Setup view selection
    m_ui->selectedPlot->setId(m_ui->crbutton, 0);
    m_ui->selectedPlot->setId(m_ui->crrbutton, 1);
    m_ui->selectedPlot->setId(m_ui->trbutton, 2);
    m_ui->selectedPlot->setId(m_ui->trrbutton, 3);
    connect(m_ui->selectedPlot, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [=] { changePlotView(m_ui->selectedPlot->checkedId()); });
}

void CacheInspector::changePlotView(int view) {
    m_ui->plotview->setCurrentIndex(view);
}

void CacheInspector::setupCacheRequestPlot() {
    // Setup bars
    auto* hits = new QCPBars(m_ui->cacheRequestPlot->xAxis, m_ui->cacheRequestPlot->yAxis);
    hits->setName("Cache Hit");
    hits->setPen(QPen(QColor(QRgb(Colors::FoundersRock))));
    hits->setBrush(QColor(QRgb(Colors::FoundersRock)));

    auto* misses = new QCPBars(m_ui->cacheRequestPlot->xAxis, m_ui->cacheRequestPlot->yAxis);
    misses->setName("Cache Miss");
    misses->setPen(QPen(QColor(QRgb(Colors::CaliforniaGold)).lighter(170)));
    misses->setBrush(QColor(QRgb(Colors::CaliforniaGold)));

    // Stack hit bars on top of miss bars
    setupBar(hits);
    setupBar(misses);
    misses->moveAbove(hits);

    // Setup ticker
    QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
    QVector<double> ticks = {1, 2, 3};
    ticker->addTicks(ticks, QVector<QString>() << "L1"
                                               << "L2"
                                               << "L3");
    m_ui->cacheRequestPlot->xAxis->setTicker(ticker);
    m_ui->cacheRequestPlot->xAxis->setRange(0, 4);
    m_ui->cacheRequestPlot->xAxis->setLabel("Cache level");
    m_ui->cacheRequestPlot->yAxis->setLabel("Cache requests");

    QFont font;
    font.setPointSize(10);
    m_ui->cacheRequestPlot->yAxis->setLabelFont(font);
    m_ui->cacheRequestPlot->xAxis->setTickLabelFont(font);
    m_ui->cacheRequestPlot->xAxis->setLabelFont(font);

    // Setup legend
    m_ui->cacheRequestPlot->legend->setVisible(true);

    // Set data
    QVector<double> hitData;
    QVector<double> missData;
    for (int i = 0; i < 3; i++) {
        hitData.push_back(m_hitData[i].last());
        missData.push_back(m_missData[i].last());
    }

    hits->setData(ticks, hitData);
    misses->setData(ticks, missData);

    m_ui->cacheRequestPlot->yAxis->rescale(true);
}
void CacheInspector::setupBar(QCPBars* bar) {
    bar->setAntialiased(false);
    bar->setStackingGap(1);
}

void CacheInspector::updateData(int /*value*/, dataRole /*role*/, cacheLevel /*level*/) {
    /*
      if (role == dataRole::miss) {
      m_missData[level] = value;
    } else {
      m_hitData[level] = value;
    }
    */
}

void CacheInspector::setupCacheRequestRatioPlot() {
    // Setup bars
    auto* ratio = new QCPBars(m_ui->cacheRequestRatioPlot->xAxis, m_ui->cacheRequestRatioPlot->yAxis);
    ratio->setName("Miss ratio");
    ratio->setPen(QPen(QColor(QRgb(Colors::FoundersRock)).lighter(170)));
    ratio->setBrush(QColor(QRgb(Colors::FoundersRock)));

    // Setup ticker
    QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
    QVector<double> ticks = {1, 2, 3};
    ticker->addTicks(ticks, QVector<QString>() << "L1"
                                               << "L2"
                                               << "L3");
    m_ui->cacheRequestRatioPlot->xAxis->setTicker(ticker);
    m_ui->cacheRequestRatioPlot->xAxis->setRange(0, 4);
    m_ui->cacheRequestRatioPlot->xAxis->setLabel("Cache level");
    m_ui->cacheRequestRatioPlot->yAxis->setLabel("Miss ratio [%]");

    QFont font;
    font.setPointSize(10);
    m_ui->cacheRequestRatioPlot->yAxis->setLabelFont(font);
    m_ui->cacheRequestRatioPlot->xAxis->setTickLabelFont(font);
    m_ui->cacheRequestRatioPlot->xAxis->setLabelFont(font);

    // Setup legend
    m_ui->cacheRequestRatioPlot->legend->setVisible(true);

    // Set data
    QVector<double> data;
    for (int i = 0; i < 3; i++) {
        data.push_back(m_missData[i].last() * 100 / (m_missData[i].last() + m_hitData[i].last()));
    }
    ratio->setData(ticks, data);
    m_ui->cacheRequestRatioPlot->yAxis->setRange(0, 100);
}
void CacheInspector::setupTemporalPlot() {}

void CacheInspector::setupTemporalRatioPlot() {
    // Preallocate graph data vector
    QVector<QVector<QCPGraphData>> graphData(3);

    for (int i = 0; i < 3; i++) {
        graphData[i].resize(m_missData[0].size());
    }

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < m_missData[j].size(); i++) {
            auto miss_data = m_missData[j][i];
            auto hit_data = m_hitData[j][i];
            graphData[j][i].key = i;
            graphData[j][i].value = ((miss_data * 100) / (miss_data + hit_data));
        }
    }

    for (int i = 0; i < 3; ++i) {
        m_ui->temporalRatioPlot->addGraph();
        QColor color = QColor(QRgb(Colors::BerkeleyBlue));
        color.setAlpha(60 + 40 * i);
        m_ui->temporalRatioPlot->graph()->setLineStyle(QCPGraph::lsLine);
        m_ui->temporalRatioPlot->graph()->setPen(QPen(color.lighter(200)));
        m_ui->temporalRatioPlot->graph()->setBrush(QBrush(color));
        m_ui->temporalRatioPlot->graph()->data()->set(graphData[i]);
        m_ui->temporalRatioPlot->graph()->setName(QString("L%1 miss ratio").arg(i + 1));
    }
    m_ui->temporalRatioPlot->yAxis->setRange(0, 100);
    m_ui->temporalRatioPlot->xAxis->rescale(true);
    m_ui->temporalRatioPlot->legend->setVisible(true);
    m_ui->temporalRatioPlot->xAxis->setLabel("Instruction count [n]");
    m_ui->temporalRatioPlot->yAxis->setLabel("Miss ratio [%]");
}

CacheInspector::~CacheInspector() {
    delete m_ui;
}
