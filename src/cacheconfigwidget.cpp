#include "cacheconfigwidget.h"
#include "ui_cacheconfigwidget.h"

#include <QSpinBox>

namespace Ripes {

CacheConfigWidget::CacheConfigWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheConfigWidget) {
    m_ui->setupUi(this);
}

void CacheConfigWidget::setCache(CacheBase* cache) {
    m_cache = cache;

    connect(m_ui->sets, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheBase::setSets);
    connect(m_ui->blocks, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheBase::setBlocks);
    connect(m_ui->lines, QOverload<int>::of(&QSpinBox::valueChanged), m_cache, &CacheBase::setLines);

    m_ui->sets->setValue(m_cache->getSetBits());
    m_ui->lines->setValue(m_cache->getLineBits());
    m_ui->blocks->setValue(m_cache->getBlockBits());
}

CacheConfigWidget::~CacheConfigWidget() {
    delete m_ui;
}

}  // namespace Ripes
