#include "cachesetupwidget.h"
#include "ui_cachesetupwidget.h"

CacheSetupWidget::CacheSetupWidget(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::CacheSetupWidget) {
  m_ui->setupUi(this);
}

CacheSetupWidget::~CacheSetupWidget() { delete m_ui; }

void CacheSetupWidget::on_groupbox_toggled(bool state) {
  emit groupBoxToggled(state);
}

void CacheSetupWidget::enable(bool state) {
  m_ui->groupbox->setEnabled(state);
  m_ui->groupbox->setChecked(false);
}

void CacheSetupWidget::setName(QString name) { m_ui->groupbox->setTitle(name); }
