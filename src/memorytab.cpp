#include "memorytab.h"
#include "ui_memorytab.h"

#include "registerwidget.h"

#include <algorithm>

MemoryTab::MemoryTab(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::MemoryTab) {
  m_ui->setupUi(this);
}

void MemoryTab::init() {
  Q_ASSERT(m_memoryPtr != nullptr && m_regPtr != nullptr);
  initializeMemoryView();

  // Initialize register ABInames
  QStringList ABInames = QStringList() << "zero"
                                       << "ra"
                                       << "sp"
                                       << "gp"
                                       << "tp"
                                       << "t0"
                                       << "t1"
                                       << "t2"
                                       << "s0/fp"
                                       << "s1"
                                       << "a0"
                                       << "a1"
                                       << "a2"
                                       << "a3"
                                       << "a4"
                                       << "a5"
                                       << "a6"
                                       << "a7"
                                       << "s2"
                                       << "s3"
                                       << "s4"
                                       << "s5"
                                       << "s6"
                                       << "s7"
                                       << "s8"
                                       << "s9"
                                       << "s10"
                                       << "s11"
                                       << "t3"
                                       << "t4"
                                       << "t5"
                                       << "t6";
  QStringList descriptions =
      QStringList() << "Hard-Wired zero"
                    << "Return Address \nSaver: Caller"
                    << "Stack pointer\nSaver: Callee"
                    << "Global pointer"
                    << "Thread pointer"
                    << "Temporary/alternate link register\nSaver: Caller"
                    << "Temporary\nSaver: Caller"
                    << "Temporary\nSaver: Caller"
                    << "Saved register/frame pointer\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Function argument/return value\nSaver: Caller"
                    << "Function argument/return value\nSaver: Caller"
                    << "Function argument\nSaver: Caller"
                    << "Function argument\nSaver: Caller"
                    << "Function argument\nSaver: Caller"
                    << "Function argument\nSaver: Caller"
                    << "Function argument\nSaver: Caller"
                    << "Function argument\nSaver: Caller"
                    << "Saved register\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Saved register\nSaver: Callee"
                    << "Temporary register\nSaver: Caller"
                    << "Temporary register\nSaver: Caller"
                    << "Temporary register\nSaver: Caller"
                    << "Temporary register\nSaver: Caller";

  // Initialize 32 register widgets
  for (int i = 0; i < 32; i++) {
    auto reg = new RegisterWidget(this);
    m_regWidgetPtrs.push_back(reg);
    reg->setRegPtr(&(*m_regPtr)[i]);
    reg->setAlias(ABInames[i]);
    reg->setNumber(i);
    reg->setToolTip(descriptions[i]);
    reg->setDisplayType(m_ui->registerdisplaytype->currentText());
    connect(m_ui->registerdisplaytype, &QComboBox::currentTextChanged,
            [=](const QString &text) { reg->setDisplayType(text); });
    m_ui->registerLayout->addWidget(reg);
  }
  m_regWidgetPtrs[0]->setEnabled(false);
}

void MemoryTab::updateRegisterWidget(int n) {
  // Invoked when runner edits an internal register, requiring an update of the
  // widget
  m_regWidgetPtrs[n]->setText();
}

void MemoryTab::initializeMemoryView() {
  std::vector<std::pair<uint32_t, uint8_t>> sortedMemory(m_memoryPtr->begin(),
                                                         m_memoryPtr->end());
  std::sort(sortedMemory.begin(), sortedMemory.end(),
            [](std::pair<uint32_t, uint8_t> a, std::pair<uint32_t, uint8_t> b) {
              return a.first > b.first;
            });
  m_ui->memoryView->setRowCount(sortedMemory.size());
  m_ui->memoryView->setColumnCount(2);
  m_ui->memoryView->setSortingEnabled(false);
  m_ui->memoryView->setHorizontalHeaderLabels(QStringList() << "Address"
                                                            << "Value");
  m_ui->memoryView->verticalHeader()->setVisible(false);
  int i = 0;
  for (const auto &entry : sortedMemory) {
    auto addr = new QTableWidgetItem(QString().setNum(entry.first, 16));
    auto val = new QTableWidgetItem(QString("%1").arg(entry.second));
    m_ui->memoryView->setItem(i, 0, addr);
    m_ui->memoryView->setItem(i, 1, val);
    i++;
  }
}
MemoryTab::~MemoryTab() { delete m_ui; }
