#include "processortab.h"
#include "ui_processortab.h"

ProcessorTab::ProcessorTab(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::ProcessorTab) {
  m_ui->setupUi(this);

  connect(m_ui->pipeline, &QRadioButton::toggled, m_ui->forwarding,
          &QCheckBox::setEnabled);
}

ProcessorTab::~ProcessorTab() { delete m_ui; }
