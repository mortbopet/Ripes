#include "memorytab.h"
#include "ui_memorytab.h"

#include "registerwidget.h"

MemoryTab::MemoryTab(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::MemoryTab) {
  m_ui->setupUi(this);

  // create some registers
  for (int i = 0; i < 32; i++) {
    m_ui->registerLayout->addWidget(new RegisterWidget(this));
  }
}

MemoryTab::~MemoryTab() { delete m_ui; }
