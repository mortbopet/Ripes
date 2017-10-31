#include "cachetab.h"
#include "ui_cachetab.h"

#include "QLineEdit"

CacheTab::CacheTab(QWidget *parent) : QWidget(parent), m_ui(new Ui::CacheTab) {
  m_ui->setupUi(this);

  setupWidgets();
  connectWidgets();
}

void CacheTab::setupWidgets() {
  // set cache size combobox values
  QStringList cacheSizes = QStringList() << "32 Bytes"
                                         << "64 Bytes"
                                         << "128 Bytes"
                                         << "256 Bytes";
  m_ui->l1size->addItems(cacheSizes);
  m_ui->l2size->addItems(cacheSizes);
  m_ui->l3size->addItems(cacheSizes);

  m_ui->l1delay->setSuffix((" cycles"));
  m_ui->l2delay->setSuffix((" cycles"));
  m_ui->l3delay->setSuffix((" cycles"));
}

void CacheTab::connectWidgets() { // Connect cache selection checkboxes
  connect(m_ui->l1checkbox, &QCheckBox::toggled, this,
          &CacheTab::cacheCountChanged);
  connect(m_ui->l2checkbox, &QCheckBox::toggled, this,
          &CacheTab::cacheCountChanged);
  connect(m_ui->l3checkbox, &QCheckBox::toggled, this,
          &CacheTab::cacheCountChanged);
}

void CacheTab::disableCacheWidget(int cacheNumber) {
  // Disables all widgets associated with a cache number
  auto disableFct = [=](QCheckBox *checkbox, QComboBox *combobox,
                        QSpinBox *lineedit) {
    checkbox->setEnabled(false);
    checkbox->setChecked(false);
    combobox->setEnabled(false);
    lineedit->setEnabled(false);
  };
  switch (cacheNumber) {
  case 1:
    disableFct(m_ui->l1checkbox, m_ui->l1size, m_ui->l1delay);
    break;
  case 2:
    disableFct(m_ui->l2checkbox, m_ui->l2size, m_ui->l2delay);
    break;
  case 3:
    disableFct(m_ui->l3checkbox, m_ui->l3size, m_ui->l3delay);
    break;
  }
}

void CacheTab::enableCacheWidget(int cacheNumber) {
  // Enables all widgets associated with a cache number
  auto enableFct = [=](QCheckBox *checkbox, QComboBox *combobox,
                       QSpinBox *lineedit) {
    checkbox->setEnabled(true);
    combobox->setEnabled(true);
    lineedit->setEnabled(true);
  };
  switch (cacheNumber) {
  case 1:
    enableFct(m_ui->l1checkbox, m_ui->l1size, m_ui->l1delay);
    break;
  case 2:
    enableFct(m_ui->l2checkbox, m_ui->l2size, m_ui->l2delay);
    break;
  case 3:
    enableFct(m_ui->l3checkbox, m_ui->l3size, m_ui->l3delay);
    break;
  }
}

void CacheTab::cacheCountChanged(bool state) {
  // Set cache selection checkboxes based on

  // CHANGE ALL THIS STUFF TO JUST CONNECT The checkbox::changed signal to
  // enable/disable slot of the other widgets.
  // and then only do check/uncheck in the below checks
  auto checkbox = dynamic_cast<QCheckBox *>(sender());
  if (checkbox == m_ui->l1checkbox) {
    if (state) {
      enableCacheWidget(2);
    } else {
      disableCacheWidget(2);
      disableCacheWidget(3);
    }
  } else if (checkbox == m_ui->l2checkbox) {
    if (state) {
      enableCacheWidget(3);
    } else {
      disableCacheWidget(3);
    }
  } else if (checkbox == m_ui->l3checkbox) {
    // nothing
  }
}

CacheTab::~CacheTab() { delete m_ui; }
