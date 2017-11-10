#include "memorytab.h"
#include "ui_memorytab.h"

#include "registerwidget.h"

#include <QWheelEvent>
#include <algorithm>

MemoryTab::MemoryTab(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::MemoryTab) {
  m_ui->setupUi(this);

  // Add display types to display comboboxes
  for (const auto &type : displayTypes.keys()) {
    m_ui->registerdisplaytype->insertItem(0, type, displayTypes[type]);
    m_ui->memorydisplaytype->insertItem(0, type, displayTypes[type]);
  }

  // Add goto values to goto combobox
  m_ui->gotoCombobox->addItem("Select", 0);
  m_ui->gotoCombobox->addItem("Text", 0);
  m_ui->gotoCombobox->addItem("Stack", 0x7ffffff0);

  m_ui->registerdisplaytype->setCurrentIndex(displayTypeN::Hex);
  m_ui->memorydisplaytype->setCurrentIndex(displayTypeN::Hex);
}

void MemoryTab::init() {
  Q_ASSERT(m_memoryPtr != nullptr && m_regPtr != nullptr);
  initializeMemoryView();
  initializeRegisterView();
}

void MemoryTab::initializeRegisterView() {
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
    reg->setDisplayType(
        qvariant_cast<displayTypeN>(m_ui->registerdisplaytype->currentData()));
    connect(m_ui->registerdisplaytype,
            QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            [=] {
              reg->setDisplayType(qvariant_cast<displayTypeN>(
                  m_ui->registerdisplaytype->currentData()));
            });
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
  m_model = new MemoryModel(m_memoryPtr);
  m_delegate = new MemoryDisplayDelegate();
  m_ui->memoryView->setModel(m_model);
  m_ui->memoryView->horizontalHeader()->setStretchLastSection(true);
  m_ui->memoryView->verticalHeader()->hide();

  // Only set delegate on byte columns
  m_ui->memoryView->setItemDelegateForColumn(1, m_delegate);
  m_ui->memoryView->setItemDelegateForColumn(2, m_delegate);
  m_ui->memoryView->setItemDelegateForColumn(3, m_delegate);
  m_ui->memoryView->setItemDelegateForColumn(4, m_delegate);

  // Memory display type
  connect(m_ui->memorydisplaytype, &QComboBox::currentTextChanged, m_delegate,
          [=] {
            m_delegate->setDisplayType(qvariant_cast<displayTypeN>(
                m_ui->memorydisplaytype->currentData()));
            m_ui->memoryView->viewport()->repaint();
          });
  m_delegate->setDisplayType(
      qvariant_cast<displayTypeN>(m_ui->memorydisplaytype->currentData()));

  connect(m_ui->memoryUp, &QPushButton::clicked, [=] {
    m_model->offsetCentralAddress(4);
    m_ui->memoryView->viewport()->repaint();
  });
  connect(m_ui->memoryDown, &QPushButton::clicked, [=] {
    m_model->offsetCentralAddress(-4);
    m_ui->memoryView->viewport()->repaint();
  });

  // Connect scroll events to the model
  connect(m_ui->memoryView, &MemoryView::scrolled, [=](bool dir) {
    if (dir) {
      m_model->offsetCentralAddress(4);
    } else {
      m_model->offsetCentralAddress(-4);
    };
  });

  // Disable editing
  m_ui->memoryView->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // Connect goto combobox
  connect(m_ui->gotoCombobox, &QComboBox::currentTextChanged, [=] {
    m_model->jumpToAddress(m_ui->gotoCombobox->currentData().toUInt());

    // Switch back to "select"
    m_ui->gotoCombobox->blockSignals(true);
    m_ui->gotoCombobox->setCurrentIndex(0);
    m_ui->gotoCombobox->blockSignals(false);
  });
}

MemoryTab::~MemoryTab() { delete m_ui; }
