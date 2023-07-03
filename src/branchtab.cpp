#include "branchtab.h"
#include "ui_branchtab.h"

#include <QPushButton>
#include <QTimer>
#include <QColor>
#include <QStyle>

#include "processorhandler.h"
#include "ripessettings.h"
#include "processors/RISC-V/rv5s_br/rv5s_branchunit.h"
#include "processors/RISC-V/rv5s_br/rv5s_br.h"

#include <cmath>
#include <typeinfo>
#include <cstdio>

namespace Ripes {
  
static QString convertToSIUnits(const double l_value, int precision = 2) {
  QString unit;
  double value;

  if (l_value < 0) {
    value = l_value * -1;
  } else {
    value = l_value;
  }

  if (value >= 1000000 && value < 1000000000) {
    value = value / 1000000;
    unit = "M";
  } else if (value >= 1000 && value < 1000000) {
    value = value / 1000;
    unit = "K";
  } else if (value >= 1 && value < 1000) {
    value = value * 1;
  } else if ((value * 1000) >= 1 && value < 1000) {
    value = value * 1000;
    unit = "m";
  } else if ((value * 1000000) >= 1 && value < 1000000) {
    value = value * 1000000;
    unit = QChar(0x00B5);
  } else if ((value * 1000000000) >= 1 && value < 1000000000) {
    value = value * 1000000000;
    unit = "n";
  }

  if (l_value > 0) {
    return (QString::number(value, 10, precision) + " " + unit);
  } else if (l_value < 0) {
    return (QString::number(value * -1, 10, precision) + " " + unit);
  }
  return QString::number(0) + " ";
}

static bool isBranch(QString inst) {
  if (inst.contains("beq",  Qt::CaseInsensitive) ||
      inst.contains("bne",  Qt::CaseInsensitive) ||
      inst.contains("blt",  Qt::CaseInsensitive) ||
      inst.contains("bge",  Qt::CaseInsensitive) ||
      inst.contains("bltu", Qt::CaseInsensitive) ||
      inst.contains("bgeu", Qt::CaseInsensitive)) {
    return true;
  }
  return false;
}

static bool isJAL(QString inst) {
  if (inst.contains("jal",  Qt::CaseInsensitive)) {
    return true;
  }
  return false;
}

static bool isJALR(QString inst) {
  if (inst.contains("jalr",  Qt::CaseInsensitive)) {
    return true;
  }
  return false;
}

static bool isRV5S_BR() {
  if (ProcessorHandler::getID() == RV32_5S_BR || ProcessorHandler::getID() == RV64_5S_BR) {
    return true;
  }
  return false;
}

static uint8_t countOnes(uint16_t num) {
  uint8_t numOnes = 0;
  for (;num != 0; num = num >> 1) {
    if (num & 1) {
      numOnes++;
    }
  }
  return numOnes;
}

template <unsigned XLEN, typename XLEN_T>
static vsrtl::core::BranchUnit<XLEN>* getBU() {
  using namespace vsrtl;
  using namespace core;
  const RV5S_BR<XLEN_T>* proc = dynamic_cast<const RV5S_BR<XLEN_T>*>(ProcessorHandler::getProcessor());
  return proc->brunit;
}

static unsigned getXLEN() {
  return ProcessorHandler::getProcessor()->implementsISA()->bits();
}

static void resetPredictorCounters() {
  if (!isRV5S_BR()) {
    return;
  }
  unsigned const XLEN = getXLEN();
  if (XLEN == 32) {
    vsrtl::core::BranchUnit<32>* brunit = getBU<32, uint32_t>();
    brunit->resetPredictorCounters();
  }
  if (XLEN == 64) {
    vsrtl::core::BranchUnit<64>* brunit = getBU<64, uint64_t>();
    brunit->resetPredictorCounters();
  }
}

static void resetPredictorState() {
  if (!isRV5S_BR()) {
    return;
  }
  unsigned const XLEN = getXLEN();
  if (XLEN == 32) {
    vsrtl::core::BranchUnit<32>* brunit = getBU<32, uint32_t>();
    brunit->resetPredictorState();
    resetPredictorCounters();
  }
  if (XLEN == 64) {
    vsrtl::core::BranchUnit<64>* brunit = getBU<64, uint64_t>();
    brunit->resetPredictorState();
    resetPredictorCounters();
  }
}

static uint16_t getNumBranch() {
  if (!isRV5S_BR()) {
    return 0;
  }
  unsigned const XLEN = getXLEN();
  if (XLEN == 32) {
    vsrtl::core::BranchUnit<32>* brunit = getBU<32, uint32_t>();
    return brunit->num_branch;
  }
  if (XLEN == 64) {
    vsrtl::core::BranchUnit<64>* brunit = getBU<64, uint64_t>();
    return brunit->num_branch;
  }
  return 0;
}

static uint16_t getNumBranchMiss() {
  if (!isRV5S_BR()) {
    return 0;
  }
  unsigned const XLEN = getXLEN();
  if (XLEN == 32) {
    vsrtl::core::BranchUnit<32>* brunit = getBU<32, uint32_t>();
    return brunit->num_branch_miss;
  }
  if (XLEN == 64) {
    vsrtl::core::BranchUnit<64>* brunit = getBU<64, uint64_t>();
    return brunit->num_branch_miss;
  }
  return 0;
}

static uint16_t* getLocalHistoryTable() {
  if (!isRV5S_BR()) {
    return nullptr;
  }
  unsigned const XLEN = getXLEN();
  if (XLEN == 32) {
    vsrtl::core::BranchUnit<32>* brunit = getBU<32, uint32_t>();
    return brunit->local_history_table;
  }
  if (XLEN == 64) {
    vsrtl::core::BranchUnit<64>* brunit = getBU<64, uint64_t>();
    return brunit->local_history_table;
  }
  return nullptr;
}

static uint16_t* getPatternHistoryTable() {
  if (!isRV5S_BR()) {
    return nullptr;
  }
  unsigned const XLEN = getXLEN();
  if (XLEN == 32) {
    vsrtl::core::BranchUnit<32>* brunit = getBU<32, uint32_t>();
    return brunit->branch_prediction_table;
  }
  if (XLEN == 64) {
    vsrtl::core::BranchUnit<64>* brunit = getBU<64, uint64_t>();
    return brunit->branch_prediction_table;
  }
  return nullptr;
}

static bool getPredictTaken() {
  if (!isRV5S_BR()) {
    return false;
  }
  unsigned const XLEN = getXLEN();
  if (XLEN == 32) {
    vsrtl::core::BranchUnit<32>* brunit = getBU<32, uint32_t>();
    return brunit->curr_pre_take.uValue();
  }
  if (XLEN == 64) {
    vsrtl::core::BranchUnit<64>* brunit = getBU<64, uint64_t>();
    return brunit->curr_pre_take.uValue();
  }
  return false;
}

BranchTab::BranchTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), m_ui(new Ui::BranchTab) {
  m_ui->setupUi(this);

  // During processor running, it should not be possible to interact with the
  // cache tab
  connect(ProcessorHandler::get(), &ProcessorHandler::procStateChangedNonRun,
          this, [=] { this->updateStatistics();
                      this->updateRuntimeFacts();
                      this->updateTables(); });
  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged,
          this, &BranchTab::procChanged);
  connect(m_ui->resetCounter, &QPushButton::clicked, this,
          [=] { resetPredictorCounters();
                this->updateStatistics();
                this->updateRuntimeFacts();
                this->updateTables(); });
  connect(m_ui->resetState, &QPushButton::clicked, this,
          [=] { resetPredictorState();
                this->updateStatistics();
                this->updateRuntimeFacts();
                this->updateTables(); });
  connect(m_ui->predictorSelect, &QComboBox::currentIndexChanged,
          this, &BranchTab::predictorChanged);
  

  // Setup statistics update timer - this timer is distinct from the
  // ProcessorHandler's update timer, given that it needs to run during
  // 'running' the processor.
  m_statUpdateTimer = new QTimer(this);
  m_statUpdateTimer->setInterval(
      1000.0 / RipesSettings::value(RIPES_SETTING_UIUPDATEPS).toInt());
  connect(m_statUpdateTimer, &QTimer::timeout, this,
          &BranchTab::updateStatistics);
  connect(RipesSettings::getObserver(RIPES_SETTING_UIUPDATEPS),
          &SettingObserver::modified, m_statUpdateTimer, [=] {
            m_statUpdateTimer->setInterval(
                1000.0 /
                RipesSettings::value(RIPES_SETTING_UIUPDATEPS).toInt());
          });
  connect(ProcessorHandler::get(), &ProcessorHandler::runStarted,
          this, [=] { m_statUpdateTimer->start();
                      m_ui->resetCounter->setEnabled(false);
                      m_ui->resetState->setEnabled(false);
                      m_ui->predictorSelect->setEnabled(false); });
  connect(ProcessorHandler::get(), &ProcessorHandler::runFinished,
          this, [=] { m_statUpdateTimer->stop();
                      m_ui->resetCounter->setEnabled(true);
                      m_ui->resetState->setEnabled(true);
                      m_ui->predictorSelect->setEnabled(true); });
  
  m_ui->splitter->setStretchFactor(0, 0);
  m_ui->splitter->setStretchFactor(1, 10);
  m_ui->splitter->setStretchFactor(2, 10);
  m_ui->splitter->setSizes({1, 1000, 1000});
  
  BranchTab::setupTables(32, 8);
  BranchTab::procChanged();
}

void BranchTab::tabVisibilityChanged(bool visible) {
  if (!m_initialized && visible) {
    m_initialized = visible;
  }
}

void BranchTab::procChanged() {
  if (!isRV5S_BR()) {
    this->setEnabled(false);
    BranchTab::updateTables();
    BranchTab::updateStatistics();
    BranchTab::updateRuntimeFacts();
    this->isBranchProc = false;
    resetPredictorState();
  }
  else {
    this->setEnabled(true);
    this->isBranchProc = true;
    BranchTab::updateTables();
    BranchTab::updateStatistics();
    BranchTab::updateRuntimeFacts();
    resetPredictorState();
  }
}

void BranchTab::predictorChanged() {
  return; // Not implemented
}

void BranchTab::updateStatistics() {
  if (!this->isBranchProc) {
    return;
  }

  m_ui->numBranch->setText(QString::number(getNumBranch()));
  m_ui->numMiss->setText(QString::number(getNumBranchMiss()));
  double accuracy = (double)(1 - (float)getNumBranchMiss() / (float)getNumBranch()) * 100.0;
  if (isnan(accuracy)) {
    accuracy = 0;
  }
  m_ui->branchAccuracy->setText(QString::number(accuracy, 'g', 5) + "%");

  static auto lastUpdateTime = std::chrono::system_clock::now();
  static long long lastCycleCount =
      ProcessorHandler::getProcessor()->getCycleCount();

  const auto timeNow = std::chrono::system_clock::now();
  const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
  const auto instrsRetired =
      ProcessorHandler::getProcessor()->getInstructionsRetired();
  const auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
                            timeNow - lastUpdateTime)
                            .count() /
                        1000.0; // in seconds
  const auto cycleDiff = cycleCount - lastCycleCount;

  // Cycle count
  m_ui->cycles->setText(QString::number(cycleCount));
  // Instructions retired
  m_ui->instrsRetired->setText(QString::number(instrsRetired));
  QString cpiText, ipcText;
  if (cycleCount != 0 && instrsRetired != 0) {
    const double cpi =
        static_cast<double>(cycleCount) / static_cast<double>(instrsRetired);
    const double ipc = 1 / cpi;
    cpiText = QString::number(cpi, 'g', 3);
    ipcText = QString::number(ipc, 'g', 3);
  }
  // CPI & IPC
  m_ui->cpi->setText(cpiText);
  m_ui->ipc->setText(ipcText);

  // Clock rate
  const double clockRate = static_cast<double>(cycleDiff) / timeDiff;
  m_ui->clockRate->setText(convertToSIUnits(clockRate) + "Hz");

  // Record timestamp values
  lastUpdateTime = timeNow;
  lastCycleCount = cycleCount;
}

void BranchTab::updateRuntimeFacts() {
  if (!this->isBranchProc) {
    return;
  }

  const RipesProcessor* proc = ProcessorHandler::getProcessor();
  AInt addr = proc->getPcForStage({0, 0});
  m_ui->currPC->setText("0x" + QString::number(addr, 16));
  QString inst = ProcessorHandler::disassembleInstr(addr);
  m_ui->currInst->setText(inst);

  if (!isRV5S_BR()) {
    m_ui->isBranch->setStyleSheet("QLineEdit"
                                  "{"
                                  "background: rgb(255, 255, 255)"
                                  "}");
    m_ui->isJump->setStyleSheet("QLineEdit"
                                "{"
                                "background: rgb(255, 255, 255)"
                                "}");
    m_ui->predictTaken->setStyleSheet("QLineEdit"
                                      "{"
                                      "background: rgb(255, 255, 255)"
                                      "}");
    m_ui->lhtEntry->setText("");
    m_ui->phtEntry->setText("");
    return;
  }

  if (isBranch(inst)) {
    m_ui->isBranch->setStyleSheet("QLineEdit"
                                  "{"
                                  "background: rgb(128, 255, 128)"
                                  "}");
    uint16_t* lht = getLocalHistoryTable();
    uint16_t* pht = getPatternHistoryTable();
    uint16_t lht_entry = lht[(addr >> 2) & ((1 << this->num_history_bits) - 1)];
    uint16_t pht_entry = pht[lht_entry];
    m_ui->lhtEntry->setText(QStringLiteral("%1").arg(lht_entry,
                                                  this->num_history_bits,
                                                  2,
                                                  QLatin1Char('0')));
    m_ui->phtEntry->setText(QStringLiteral("%1").arg(pht_entry,
                                                  this->num_prediction_bits,
                                                  2,
                                                  QLatin1Char('0')));
  }
  else {
    m_ui->isBranch->setStyleSheet("QLineEdit"
                                  "{"
                                  "background: rgb(255, 128, 128)"
                                  "}");
    m_ui->lhtEntry->setText("");
    m_ui->phtEntry->setText("");
  }

  if (isJAL(inst) || isJALR(inst)) {
    m_ui->isJump->setStyleSheet("QLineEdit"
                                  "{"
                                  "background: rgb(128, 255, 128)"
                                  "}");
  }
  else {
    m_ui->isJump->setStyleSheet("QLineEdit"
                                  "{"
                                  "background: rgb(255, 128, 128)"
                                  "}");
  }

  if (getPredictTaken()) {
    m_ui->predictTaken->setStyleSheet("QLineEdit"
                                      "{"
                                      "background: rgb(128, 255, 128)"
                                      "}");
  }
  else {
    m_ui->predictTaken->setStyleSheet("QLineEdit"
                                      "{"
                                      "background: rgb(255, 128, 128)"
                                      "}");
  }
}

void BranchTab::setupTables(int rows, int columns) {
  if (!this->isBranchProc) {
    return;
  }

  this->table_rows = rows;
  this->table_columns = columns;

  m_ui->table1->setRowCount(rows + 1);
  m_ui->table1->setColumnCount(columns + 1);
  for (int i = 0; i < rows + 1; i++) {
    int h = m_ui->table1->rowHeight(i);
    m_ui->table1->setRowHeight(i, h - 10);
  }
  for (int i = 1; i < rows + 1; i++) {
    QTableWidgetItem* item = new QTableWidgetItem("0x" + QString::number((i - 1) * columns * 4, 16));
    item->setTextAlignment(Qt::AlignRight);
    m_ui->table1->setItem(i, 0, item);
  }
  for (int i = 0; i < columns + 1; i++) {
    int w = m_ui->table1->columnWidth(i);
    m_ui->table1->setColumnWidth(i, w - 30);
  }
  for (int i = 1; i < columns + 1; i++) {
    QTableWidgetItem* item = new QTableWidgetItem("0x" + QString::number((i - 1) * 4, 16));
    item->setTextAlignment(Qt::AlignHCenter);
    m_ui->table1->setItem(0, i, item);
  }

  m_ui->table2->setRowCount(rows + 1);
  m_ui->table2->setColumnCount(columns + 1);
  for (int i = 0; i < rows + 1; i++) {
    int h = m_ui->table2->rowHeight(i);
    m_ui->table2->setRowHeight(i, h - 10);
  }
  for (int i = 1; i < rows + 1; i++) {
    QTableWidgetItem* item = new QTableWidgetItem(QStringLiteral("%1").arg((i - 1) * columns,
                                                  (int)log2(rows * columns),
                                                  2,
                                                  QLatin1Char('0')));
    item->setTextAlignment(Qt::AlignRight);
    m_ui->table2->setItem(i, 0, item);
  }
  for (int i = 0; i < columns + 1; i++) {
    int w = m_ui->table2->columnWidth(i);
    m_ui->table2->setColumnWidth(i, w - 30);
  }
  for (int i = 1; i < columns + 1; i++) {
    QTableWidgetItem* item = new QTableWidgetItem(QStringLiteral("%1").arg((i - 1),
                                                  (int)log2(columns),
                                                  2,
                                                  QLatin1Char('0')));
    item->setTextAlignment(Qt::AlignHCenter);
    m_ui->table2->setItem(0, i, item);
  }
}

void BranchTab::updateTables() {
  if (!this->isBranchProc) {
    return;
  }

  QBrush brush_snt(QColor(255, 128, 128));
  QBrush brush_wnt(QColor(255, 192, 192));
  QBrush brush_wt(QColor(192, 255, 192));
  QBrush brush_st(QColor(128, 255, 128));

  uint16_t* lht = getLocalHistoryTable();
  uint16_t* pht = getPatternHistoryTable();

  if (!lht || !pht) {
    for (int i = 1; i < this->table_rows + 1; i++) {
      for (int j = 1; j < this->table_columns + 1; j++) {
        uint16_t lht_entry = 0;
        uint16_t pht_entry = 0;

        QTableWidgetItem* item1 = new QTableWidgetItem(QStringLiteral("%1").arg(lht_entry,
                                                                          this->num_history_bits,
                                                                          2,
                                                                          QLatin1Char('0')));
        item1->setTextAlignment(Qt::AlignHCenter);
        m_ui->table1->setItem(i, j, item1);

        QTableWidgetItem* item2 = new QTableWidgetItem(QStringLiteral("%1").arg(pht_entry,
                                                                          this->num_prediction_bits,
                                                                          2,
                                                                          QLatin1Char('0')));
        item2->setTextAlignment(Qt::AlignHCenter);
        m_ui->table2->setItem(i, j, item2);
      }
    }
  }
  else {
    for (int i = 1; i < this->table_rows + 1; i++) {
      for (int j = 1; j < this->table_columns + 1; j++) {
        uint16_t lht_entry = lht[(j - 1) + this->table_columns * (i - 1)];
        uint16_t pht_entry = pht[(j - 1) + this->table_columns * (i - 1)];

        QTableWidgetItem* item1 = new QTableWidgetItem(QStringLiteral("%1").arg(lht_entry,
                                                                          this->num_history_bits,
                                                                          2,
                                                                          QLatin1Char('0')));
        item1->setTextAlignment(Qt::AlignHCenter);
        m_ui->table1->setItem(i, j, item1);
        switch (countOnes(lht_entry)) {
          case 0: m_ui->table1->item(i, j)->setBackground(QBrush(QColor(255, 128, 128))); break;
          case 1: m_ui->table1->item(i, j)->setBackground(QBrush(QColor(255, 160, 160))); break;
          case 2: m_ui->table1->item(i, j)->setBackground(QBrush(QColor(255, 192, 192))); break;
          case 3: m_ui->table1->item(i, j)->setBackground(QBrush(QColor(255, 224, 224))); break;
          case 4: m_ui->table1->item(i, j)->setBackground(QBrush(QColor(255, 255, 255))); break;
          case 5: m_ui->table1->item(i, j)->setBackground(QBrush(QColor(224, 255, 224))); break;
          case 6: m_ui->table1->item(i, j)->setBackground(QBrush(QColor(192, 255, 192))); break;
          case 7: m_ui->table1->item(i, j)->setBackground(QBrush(QColor(160, 255, 160))); break;
          case 8: m_ui->table1->item(i, j)->setBackground(QBrush(QColor(128, 255, 128))); break;
        }

        QTableWidgetItem* item2 = new QTableWidgetItem(QStringLiteral("%1").arg(pht_entry,
                                                                          this->num_prediction_bits,
                                                                          2,
                                                                          QLatin1Char('0')));
        item2->setTextAlignment(Qt::AlignHCenter);
        m_ui->table2->setItem(i, j, item2);
        switch (pht_entry) {
          case 0: m_ui->table2->item(i, j)->setBackground(brush_snt); break;
          case 1: m_ui->table2->item(i, j)->setBackground(brush_wnt); break;
          case 2: m_ui->table2->item(i, j)->setBackground(brush_wt); break;
          case 3: m_ui->table2->item(i, j)->setBackground(brush_st); break;
        }
      }
    }
  }
}

BranchTab::~BranchTab() { delete m_ui; }

} // namespace Ripes
