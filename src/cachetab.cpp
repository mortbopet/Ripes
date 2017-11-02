#include "cachetab.h"
#include "cachesetupwidget.h"
#include "ui_cachetab.h"

#include "QGroupBox"

CacheTab::CacheTab(QWidget *parent) : QWidget(parent), m_ui(new Ui::CacheTab) {
  m_ui->setupUi(this);

  setupWidgets();
  connectWidgets();
}

void CacheTab::setupWidgets() {
  m_ui->l1->enable(true);
  m_ui->l1->setName("L1 cache");
  m_ui->l2->setName("L2 cache");
  m_ui->l3->setName("L3 cache");
}

void CacheTab::connectWidgets() { // Connect cache selection checkboxes
  connect(m_ui->l1, &CacheSetupWidget::groupBoxToggled, this,
          &CacheTab::cacheCountChanged);
  connect(m_ui->l2, &CacheSetupWidget::groupBoxToggled, this,
          &CacheTab::cacheCountChanged);
  connect(m_ui->l3, &CacheSetupWidget::groupBoxToggled, this,
          &CacheTab::cacheCountChanged);
}

void CacheTab::cacheCountChanged(bool state) {
  // Set cache selection checkboxes based on

  // CHANGE ALL THIS STUFF TO JUST CONNECT The checkbox::changed signal to
  // enable/disable slot of the other widgets.
  // and then only do check/uncheck in the below checks
  auto cachewidget = dynamic_cast<CacheSetupWidget *>(sender());
  if (cachewidget == m_ui->l1) {
    if (state) {
      m_ui->l2->enable(true);
    } else {
      m_ui->l2->enable(false);
      m_ui->l3->enable(false);
    }
  } else if (cachewidget == m_ui->l2) {
    if (state) {
      m_ui->l3->enable(true);
    } else {
      m_ui->l3->enable(false);
    }
  } else if (cachewidget == m_ui->l3) {
    // nothing
  }
}

CacheTab::~CacheTab() { delete m_ui; }
