#include "cacheinspector.h"
#include "ui_cacheinspector.h"

#include "qcustomplot.h"

CacheInspector::CacheInspector(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::CacheInspector) {
  m_ui->setupUi(this);

  // Set initial data
  m_hitData = {5, 7, 9};
  m_missData = {2, 0, 5};

  setupCacheRequestPlot();
  setupCacheRequestRatioPlot();

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
  QCPBars *hits =
      new QCPBars(m_ui->cacheRequestPlot->xAxis, m_ui->cacheRequestPlot->yAxis);
  hits->setName("Cache hit");
  hits->setPen(QPen(QColor(136, 240, 0).lighter(170)));
  hits->setBrush(QColor(136, 240, 0));

  QCPBars *misses =
      new QCPBars(m_ui->cacheRequestPlot->xAxis, m_ui->cacheRequestPlot->yAxis);
  misses->setName("Cache Miss");
  misses->setPen(QPen(QColor(255, 35, 0).lighter(170)));
  misses->setBrush(QColor(255, 35, 0));

  // Stack hit bars on top of miss bars
  setupBar(hits);
  setupBar(misses);
  misses->moveAbove(hits);

  // Setup ticker
  QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
  QVector<double> ticks = {1, 2, 3};
  ticker->addTicks(ticks,
                   QVector<QString>() << "L1"
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
  hits->setData(ticks, m_hitData);
  misses->setData(ticks, m_missData);

  m_ui->cacheRequestPlot->yAxis->rescale(true);
}
void CacheInspector::setupBar(QCPBars *bar) {
  bar->setAntialiased(false);
  bar->setStackingGap(1);
}

void CacheInspector::updateData(int value, dataRole role, cacheLevel level) {
  if (role == dataRole::miss) {
    m_missData[level] = value;
  } else {
    m_hitData[level] = value;
  }
}

void CacheInspector::setupCacheRequestRatioPlot() {
  // Setup bars
  QCPBars *ratio = new QCPBars(m_ui->cacheRequestRatioPlot->xAxis,
                               m_ui->cacheRequestRatioPlot->yAxis);
  ratio->setName("Miss ratio");
  ratio->setPen(QPen(QColor(179, 230, 249).lighter(170)));
  ratio->setBrush(QColor(179, 230, 249));

  // Setup ticker
  QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
  QVector<double> ticks = {1, 2, 3};
  ticker->addTicks(ticks,
                   QVector<QString>() << "L1"
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
  for (int i = 0; i < m_missData.size(); i++) {
    data.push_back(m_missData[i] * 100 / (m_missData[i] + m_hitData[i]));
  }
  ratio->setData(ticks, data);
  m_ui->cacheRequestRatioPlot->yAxis->setRange(0, 100);
}
void CacheInspector::setupTemporalPlot() {}
void CacheInspector::setupTemporalRatioPlot() {}

CacheInspector::~CacheInspector() { delete m_ui; }
