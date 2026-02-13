#include "branchtab.h"
#include "ui_branchtab.h"

#include <QColor>
#include <QPushButton>
#include <QStyle>
#include <QTimer>

#include "processorhandler.h"
#include "processors/branch/predictorhandler.h"
#include "processors/branch/branchpredictionprocessor.h"
#include "ripessettings.h"

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

static bool isConditional() {
  BranchPredictionProcessor *proc = dynamic_cast<BranchPredictionProcessor *>(
      ProcessorHandler::getProcessorNonConst());
  return proc->currentInstructionIsConditional();
}

static bool isBranch() {
  BranchPredictionProcessor *proc = dynamic_cast<BranchPredictionProcessor *>(
      ProcessorHandler::getProcessorNonConst());
  return proc->currentInstructionIsBranch();
}

static bool isBR() {
  return ProcessorHandler::getProcessorNonConst()->supportsBranchPrediction();
}

static uint8_t countOnes(uint16_t num) {
  uint8_t numOnes = 0;
  for (; num != 0; num = num >> 1) {
    if (num & 1) {
      numOnes++;
    }
  }
  return numOnes;
}

static uint16_t getNumConditional() {
  return PredictorHandler::getPredictor()->num_conditional;
}

static uint16_t getNumConditionalMiss() {
  return PredictorHandler::getPredictor()->num_conditional_miss;
}

static double getConditionalAccuracy() {
  return PredictorHandler::getPredictor()->getConditionalAccuracy();
}

static uint16_t *getLocalHistoryTable() {
  return PredictorHandler::getPredictor()->lht.get();
}

static uint16_t *getPatternHistoryTable() {
  return PredictorHandler::getPredictor()->pht.get();
}

static bool getPredictTaken() {
  BranchPredictionProcessor *proc = dynamic_cast<BranchPredictionProcessor *>(
      ProcessorHandler::getProcessorNonConst());
  return proc->currentGetPrediction();
}

static void resetPredictorCounters() {
  PredictorHandler::getPredictor()->resetPredictorCounters();
}

static void resetPredictorState() {
  PredictorHandler::getPredictor()->resetPredictorState();
  PredictorHandler::getPredictor()->resetPredictorCounters();
}

static void changePredictor(uint8_t predictor, uint16_t num_address_bits,
                            uint16_t num_history_bits,
                            uint16_t num_state_bits) {
  PredictorHandler::changePredictor(predictor, num_address_bits,
                                    num_history_bits, num_state_bits);
}

BranchTab::BranchTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), m_ui(new Ui::BranchTab) {
  m_ui->setupUi(this);

  // During processor running, it should not be possible to interact with the
  // cache tab
  connect(ProcessorHandler::get(), &ProcessorHandler::procStateChangedNonRun,
          this, [=] {
            this->updateStatistics();
            this->updateRuntimeFacts();
            this->updateTables();
          });
  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this,
          &BranchTab::procChanged);
  connect(m_ui->resetCounter, &QPushButton::clicked, this, [=] {
    resetPredictorCounters();
    this->updateStatistics();
    this->updateRuntimeFacts();
    this->updateTables();
  });
  connect(m_ui->resetState, &QPushButton::clicked, this, [=] {
    resetPredictorState();
    this->updateStatistics();
    this->updateRuntimeFacts();
    this->updateTables();
  });
  connect(m_ui->predictorSelect, &QComboBox::currentIndexChanged, this,
          [=] { this->predictorChanged(true); });
  connect(m_ui->address_bits, &QComboBox::currentIndexChanged, this,
          [=] { this->predictorChanged(false); });
  connect(m_ui->history_bits, &QComboBox::currentIndexChanged, this,
          [=] { this->predictorChanged(false); });
  connect(m_ui->state_bits, &QComboBox::currentIndexChanged, this,
          [=] { this->predictorChanged(false); });

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
  connect(ProcessorHandler::get(), &ProcessorHandler::runStarted, this, [=] {
    m_statUpdateTimer->start();
    m_ui->resetCounter->setEnabled(false);
    m_ui->resetState->setEnabled(false);
    m_ui->predictorSelect->setEnabled(false);
    m_ui->address_bits->setEnabled(false);
    m_ui->history_bits->setEnabled(false);
    m_ui->state_bits->setEnabled(false);
  });
  connect(ProcessorHandler::get(), &ProcessorHandler::runFinished, this, [=] {
    m_statUpdateTimer->stop();
    m_ui->resetCounter->setEnabled(true);
    m_ui->resetState->setEnabled(true);
    m_ui->predictorSelect->setEnabled(true);
    m_ui->address_bits->setEnabled(true);
    m_ui->history_bits->setEnabled(true);
    m_ui->state_bits->setEnabled(true);
  });

  m_ui->splitter->setStretchFactor(0, 0);
  m_ui->splitter->setStretchFactor(1, 10);
  m_ui->splitter->setStretchFactor(2, 10);
  m_ui->splitter->setSizes({1, 1000, 1000});

  BranchTab::procChanged();
}

void BranchTab::tabVisibilityChanged(bool visible) {
  if (!m_initialized && visible) {
    m_initialized = visible;
  }
}

void BranchTab::procChanged() {
  if (!isBR()) {
    this->setEnabled(false);
    BranchTab::predictorChanged(true);
    BranchTab::updateStatistics();
    BranchTab::updateRuntimeFacts();
    this->is_branch_proc = false;
    resetPredictorState();
  } else {
    this->setEnabled(true);
    this->is_branch_proc = true;
    BranchTab::predictorChanged(true);
    BranchTab::updateStatistics();
    BranchTab::updateRuntimeFacts();
    resetPredictorState();
  }
}

void BranchTab::predictorChanged(bool is_preset) {
  if (is_preset) {
    int index = m_ui->predictorSelect->currentIndex();
    switch (index) {
    case 0:
      this->predictor = 0; // Local Predictor
      this->num_address_bits = 8;
      this->num_history_bits = 8;
      this->num_state_bits = 2;
      m_ui->address_bits->setEnabled(true);
      m_ui->history_bits->setEnabled(true);
      m_ui->state_bits->setEnabled(true);
      break;
    case 1:
      this->predictor = 1; // Global Predictor
      this->num_address_bits = 0;
      this->num_history_bits = 8;
      this->num_state_bits = 2;
      m_ui->address_bits->setEnabled(false);
      m_ui->history_bits->setEnabled(true);
      m_ui->state_bits->setEnabled(true);
      break;
    case 2:
      this->predictor = 2; // Saturating Counter
      this->num_address_bits = 0;
      this->num_history_bits = 0;
      this->num_state_bits = 2;
      m_ui->address_bits->setEnabled(false);
      m_ui->history_bits->setEnabled(false);
      m_ui->state_bits->setEnabled(true);
      break;
    case 3:
      this->predictor = 3; // Always Taken
      this->num_address_bits = 0;
      this->num_history_bits = 0;
      this->num_state_bits = 0;
      m_ui->address_bits->setEnabled(false);
      m_ui->history_bits->setEnabled(false);
      m_ui->state_bits->setEnabled(false);
      break;
    case 4:
      this->predictor = 4; // Always Not Taken
      this->num_address_bits = 0;
      this->num_history_bits = 0;
      this->num_state_bits = 0;
      m_ui->address_bits->setEnabled(false);
      m_ui->history_bits->setEnabled(false);
      m_ui->state_bits->setEnabled(false);
      break;
    }
    bool oldState = m_ui->address_bits->blockSignals(true);
    m_ui->address_bits->setCurrentIndex(this->num_address_bits);
    m_ui->address_bits->blockSignals(oldState);

    oldState = m_ui->history_bits->blockSignals(true);
    m_ui->history_bits->setCurrentIndex(this->num_history_bits);
    m_ui->history_bits->blockSignals(oldState);

    oldState = m_ui->state_bits->blockSignals(true);
    m_ui->state_bits->setCurrentIndex(this->num_state_bits);
    m_ui->state_bits->blockSignals(oldState);
  }

  else {
    this->predictor = m_ui->predictorSelect->currentIndex();
    this->num_address_bits = m_ui->address_bits->currentIndex();
    this->num_history_bits = m_ui->history_bits->currentIndex();
    this->num_state_bits = m_ui->state_bits->currentIndex();
  }

  changePredictor(this->predictor, this->num_address_bits,
                  this->num_history_bits, this->num_state_bits);

  int num_table1_entries = 1 << this->num_address_bits;
  int columns1 = (num_table1_entries > 8) ? 8 : num_table1_entries;
  int rows1 = (num_table1_entries > 8) ? num_table1_entries / columns1 : 1;

  int num_table2_entries = 1 << this->num_history_bits;
  int columns2 = (num_table2_entries > 8) ? 8 : num_table2_entries;
  int rows2 = (num_table2_entries > 8) ? num_table2_entries / columns2 : 1;

  BranchTab::setupTables(rows1, columns1, rows2, columns2);
  BranchTab::updateTables();
  BranchTab::updateStatistics();
  BranchTab::updateRuntimeFacts();
}

void BranchTab::updateStatistics() {
  if (!this->is_branch_proc) {
    return;
  }

  m_ui->numBranch->setText(QString::number(getNumConditional()));
  m_ui->numMiss->setText(QString::number(getNumConditionalMiss()));
  double accuracy = getConditionalAccuracy();
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
  if (!this->is_branch_proc) {
    return;
  }

  const RipesProcessor *proc = ProcessorHandler::getProcessor();
  AInt addr = proc->getPcForStage({0, 0});
  m_ui->currPC->setText("0x" + QString::number(addr, 16));
  QString inst = ProcessorHandler::disassembleInstr(addr);
  m_ui->currInst->setText(inst);

  if (!isBR()) {
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

  if (isConditional()) {
    m_ui->isBranch->setStyleSheet("QLineEdit"
                                  "{"
                                  "background: rgb(128, 255, 128)"
                                  "}");
    uint16_t *lht = getLocalHistoryTable();
    uint16_t *pht = getPatternHistoryTable();
    if (lht == nullptr) {
      m_ui->lhtEntry->setText("");
    }
    uint16_t lht_entry = lht[(addr >> 2) & ((1 << this->num_address_bits) - 1)];
    uint16_t pht_entry = pht[lht_entry];
    m_ui->lhtEntry->setText(QStringLiteral("%1").arg(
        lht_entry, this->num_history_bits, 2, QLatin1Char('0')));
    m_ui->phtEntry->setText(QStringLiteral("%1").arg(
        pht_entry, this->num_state_bits, 2, QLatin1Char('0')));
  } else {
    m_ui->isBranch->setStyleSheet("QLineEdit"
                                  "{"
                                  "background: rgb(255, 128, 128)"
                                  "}");
    m_ui->lhtEntry->setText("");
    m_ui->phtEntry->setText("");
  }

  if (isBranch() && !isConditional()) {
    m_ui->isJump->setStyleSheet("QLineEdit"
                                "{"
                                "background: rgb(128, 255, 128)"
                                "}");
  } else {
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
  } else {
    m_ui->predictTaken->setStyleSheet("QLineEdit"
                                      "{"
                                      "background: rgb(255, 128, 128)"
                                      "}");
  }
}

void BranchTab::setupTables(int rows1, int columns1, int rows2, int columns2) {
  if (!this->is_branch_proc) {
    return;
  }

  this->table1_rows = rows1;
  this->table1_columns = columns1;
  this->table2_rows = rows2;
  this->table2_columns = columns2;

  m_ui->table1->setRowCount(rows1 + 1);
  m_ui->table1->setColumnCount(columns1 + 1);
  for (int i = 0; i < rows1 + 1; i++) {
    m_ui->table1->setRowHeight(i, 20);
  }
  for (int i = 1; i < rows1 + 1; i++) {
    QTableWidgetItem *item = new QTableWidgetItem(
        "0x" + QString::number((i - 1) * columns1 * 4, 16));
    item->setTextAlignment(Qt::AlignRight);
    m_ui->table1->setItem(i, 0, item);
  }
  for (int i = 0; i < columns1 + 1; i++) {
    m_ui->table1->setColumnWidth(i, 70);
  }
  for (int i = 1; i < columns1 + 1; i++) {
    QTableWidgetItem *item =
        new QTableWidgetItem("0x" + QString::number((i - 1) * 4, 16));
    item->setTextAlignment(Qt::AlignHCenter);
    m_ui->table1->setItem(0, i, item);
  }

  m_ui->table2->setRowCount(rows2 + 1);
  m_ui->table2->setColumnCount(columns2 + 1);
  for (int i = 0; i < rows2 + 1; i++) {
    m_ui->table2->setRowHeight(i, 20);
  }
  for (int i = 1; i < rows2 + 1; i++) {
    QTableWidgetItem *item = new QTableWidgetItem(QStringLiteral("%1").arg(
        (i - 1) * columns2, (int)log2(rows2 * columns2), 2, QLatin1Char('0')));
    item->setTextAlignment(Qt::AlignRight);
    m_ui->table2->setItem(i, 0, item);
  }
  for (int i = 0; i < columns2 + 1; i++) {
    m_ui->table2->setColumnWidth(i, 70);
  }
  for (int i = 1; i < columns2 + 1; i++) {
    QTableWidgetItem *item = new QTableWidgetItem(QStringLiteral("%1").arg(
        (i - 1), (int)log2(columns2), 2, QLatin1Char('0')));
    item->setTextAlignment(Qt::AlignHCenter);
    m_ui->table2->setItem(0, i, item);
  }
}

void BranchTab::updateTables() {
  if (!this->is_branch_proc) {
    return;
  }

  QBrush brush_snt(QColor(255, 128, 128));
  QBrush brush_wnt(QColor(255, 192, 192));
  QBrush brush_wt(QColor(192, 255, 192));
  QBrush brush_st(QColor(128, 255, 128));

  uint16_t *lht = getLocalHistoryTable();
  uint16_t *pht = getPatternHistoryTable();

  if (!lht || !pht) {
    for (int i = 1; i < this->table1_rows + 1; i++) {
      for (int j = 1; j < this->table1_columns + 1; j++) {
        uint16_t lht_entry = 0;

        QTableWidgetItem *item = new QTableWidgetItem(QStringLiteral("%1").arg(
            lht_entry, this->num_history_bits, 2, QLatin1Char('0')));
        item->setTextAlignment(Qt::AlignHCenter);
        m_ui->table1->setItem(i, j, item);
      }
    }
    for (int i = 1; i < this->table2_rows + 1; i++) {
      for (int j = 1; j < this->table2_columns + 1; j++) {
        uint16_t pht_entry = 0;

        QTableWidgetItem *item = new QTableWidgetItem(QStringLiteral("%1").arg(
            pht_entry, this->num_state_bits, 2, QLatin1Char('0')));
        item->setTextAlignment(Qt::AlignHCenter);
        m_ui->table2->setItem(i, j, item);
      }
    }
  } else {
    for (int i = 1; i < this->table1_rows + 1; i++) {
      for (int j = 1; j < this->table1_columns + 1; j++) {
        uint16_t lht_entry = lht[(j - 1) + this->table1_columns * (i - 1)];

        QTableWidgetItem *item = new QTableWidgetItem(QStringLiteral("%1").arg(
            lht_entry, this->num_history_bits, 2, QLatin1Char('0')));
        item->setTextAlignment(Qt::AlignHCenter);

        m_ui->table1->setItem(i, j, item);

        if (this->num_history_bits % 2 == 0) {
          int num_ones = countOnes(lht_entry);
          int step = 0;
          if (this->num_history_bits == 0) {
            step = 128;
          } else {
            step = 128 / (this->num_history_bits / 2);
          }

          if (num_ones == this->num_history_bits / 2) {
            m_ui->table1->item(i, j)->setBackground(
                QBrush(QColor(255, 255, 255)));
          }

          else if (num_ones < this->num_history_bits / 2) {
            int num_steps = num_ones;
            m_ui->table1->item(i, j)->setBackground(QBrush(
                QColor(255, 128 + num_steps * step, 128 + num_steps * step)));
          }

          else {
            int num_steps = this->num_history_bits - num_ones;
            m_ui->table1->item(i, j)->setBackground(QBrush(
                QColor(128 + num_steps * step, 255, 128 + num_steps * step)));
          }
        }

        else {
          int num_ones = countOnes(lht_entry);
          int step = 128 / ((this->num_history_bits + 1) / 2);

          if (num_ones <= this->num_history_bits / 2) {
            int num_steps = num_ones;
            m_ui->table1->item(i, j)->setBackground(QBrush(
                QColor(255, 128 + num_steps * step, 128 + num_steps * step)));
          }

          else {
            int num_steps = this->num_history_bits - num_ones;
            m_ui->table1->item(i, j)->setBackground(QBrush(
                QColor(128 + num_steps * step, 255, 128 + num_steps * step)));
          }
        }
      }
    }
    for (int i = 1; i < this->table2_rows + 1; i++) {
      for (int j = 1; j < this->table2_columns + 1; j++) {
        uint16_t pht_entry = pht[(j - 1) + this->table2_columns * (i - 1)];

        QTableWidgetItem *item = new QTableWidgetItem(QStringLiteral("%1").arg(
            pht_entry, this->num_state_bits, 2, QLatin1Char('0')));
        item->setTextAlignment(Qt::AlignHCenter);

        m_ui->table2->setItem(i, j, item);

        if (this->predictor == 4) {
          m_ui->table2->item(i, j)->setBackground(
              QBrush(QColor(255, 128, 128)));
        } else {
          int step = 128 / (1 << (this->num_state_bits - 1));

          if (pht_entry < (1 << (this->num_state_bits - 1))) {
            int num_steps = pht_entry;
            m_ui->table2->item(i, j)->setBackground(QBrush(
                QColor(255, 128 + num_steps * step, 128 + num_steps * step)));
          }

          else {
            int num_steps = ((1 << this->num_state_bits) - 1) - pht_entry;
            m_ui->table2->item(i, j)->setBackground(QBrush(
                QColor(128 + num_steps * step, 255, 128 + num_steps * step)));
          }
        }
      }
    }
  }
}

BranchTab::~BranchTab() { delete m_ui; }

} // namespace Ripes
