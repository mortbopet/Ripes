#include "cacheinspector.h"
#include "ui_cacheinspector.h"

#include "qcustomplot.h"

CacheInspector::CacheInspector(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::CacheInspector) {
  m_ui->setupUi(this);

  // Set initial data
  m_hitData = {5, 7, 9};
  m_missData = {2, 0, 5};

  // Setup bars
  m_hits =
      new QCPBars(m_ui->plottingwidget->xAxis, m_ui->plottingwidget->yAxis);
  m_hits->setName("Cache hit");
  m_hits->setPen(QPen(QColor(136, 240, 0).lighter(170)));
  m_hits->setBrush(QColor(136, 240, 0));

  m_misses =
      new QCPBars(m_ui->plottingwidget->xAxis, m_ui->plottingwidget->yAxis);
  m_misses->setName("Cache Miss");
  m_misses->setPen(QPen(QColor(255, 35, 0).lighter(170)));
  m_misses->setBrush(QColor(255, 35, 0));

  // Stack hit bars on top of miss bars
  setupBar(m_hits);
  setupBar(m_misses);
  m_misses->moveAbove(m_hits);

  // Setup ticker
  QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
  QVector<double> ticks = {1, 2, 3};
  ticker->addTicks(ticks,
                   QVector<QString>() << "L1"
                                      << "L2"
                                      << "L3");
  m_ui->plottingwidget->xAxis->setTicker(ticker);
  m_ui->plottingwidget->xAxis->setRange(0, 4);
  m_ui->plottingwidget->xAxis->setLabel("Cache level");
  m_ui->plottingwidget->yAxis->setLabel("Cache requests");

  QFont font;
  font.setPointSize(10);
  m_ui->plottingwidget->yAxis->setLabelFont(font);
  m_ui->plottingwidget->xAxis->setTickLabelFont(font);
  m_ui->plottingwidget->xAxis->setLabelFont(font);

  // Setup legend
  m_ui->plottingwidget->legend->setVisible(true);

  // Set data
  m_hits->setData(ticks, m_hitData);
  m_misses->setData(ticks, m_missData);

  m_ui->plottingwidget->yAxis->rescale(true);
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

CacheInspector::~CacheInspector() { delete m_ui; }
