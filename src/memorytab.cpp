#include "memorytab.h"
#include "ui_memorytab.h"

#include "registerwidget.h"

MemoryTab::MemoryTab(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::MemoryTab) {
  m_ui->setupUi(this);

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
    reg->setAlias(ABInames[i]);
    reg->setNumber(i);
    reg->setToolTip(descriptions[i]);
    reg->setDisplayType(m_ui->registerdisplaytype->currentText());
    reg->setRegPtr(&(*m_regPtr)[i]);
    connect(m_ui->registerdisplaytype, &QComboBox::currentTextChanged,
            [=](const QString &text) { reg->setDisplayType(text); });
    m_ui->registerLayout->addWidget(reg);
  }
}

MemoryTab::~MemoryTab() { delete m_ui; }
