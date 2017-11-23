#include "cachesetupwidget.h"
#include "cachebase.h"
#include "ui_cachesetupwidget.h"

#include "defines.h"

CacheSetupWidget::CacheSetupWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheSetupWidget) {
    m_ui->setupUi(this);

    // set combobox entries
    m_ui->cachesize->setInsertPolicy(QComboBox::InsertAtBottom);
    for (const auto& entry : cacheSizes) {
        m_ui->cachesize->addItem(entry.second);
    }
    m_ui->cachetype->setInsertPolicy(QComboBox::InsertAtBottom);
    for (const auto& entry : cacheTypes) {
        m_ui->cachetype->addItem(entry.first);
    }

    // Connect widget to its m_cachePtr
    // Connect widget
    connect(m_ui->cachesize, &QComboBox::currentTextChanged, this, &CacheSetupWidget::cacheSizeChanged);
    connect(m_ui->cachedelay, QOverload<int>::of(&QSpinBox::valueChanged), this, &CacheSetupWidget::cacheDelayChanged);
}

CacheSetupWidget::~CacheSetupWidget() {
    delete m_ui;
}

void CacheSetupWidget::cacheSizeChanged(const QString& index) {
    if (m_cachePtr != nullptr) {
        // bad way!
        auto size = index.split(" ")[0].toInt();
        m_cachePtr->resize(size);
    }
}

void CacheSetupWidget::cacheDelayChanged(int delay) {
    if (m_cachePtr != nullptr) {
        m_cachePtr->setSearchPenalty(delay);
    }
}

void CacheSetupWidget::on_groupbox_toggled(bool state) {
    emit groupBoxToggled(state);
}

void CacheSetupWidget::enable(bool state) {
    m_ui->groupbox->setEnabled(state);
    m_ui->groupbox->setChecked(false);
}

void CacheSetupWidget::setName(QString name) {
    m_ui->groupbox->setTitle(name);
}
