#include "cachetabwidget.h"
#include "ui_cachetabwidget.h"

#include "memoryviewerwidget.h"

#include "memorytab.h"

namespace Ripes {

CacheTabWidget::CacheTabWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheTabWidget) {
    while (parent != nullptr) {
        auto* mt = dynamic_cast<MemoryTab*>(parent);
        if (mt) {
            m_parent = mt;
            break;
        } else {
            parent = parent->parentWidget();
        }
    }
    Q_ASSERT(m_parent);

    m_ui->setupUi(this);

    m_ui->dataCacheWidget->setType(CacheSim::CacheType::DataCache);
    m_ui->instructionCacheWidget->setType(CacheSim::CacheType::InstrCache);

    // Make selection changes in the cache trigger the memory viewer to set its central address to the selected address
    connectCacheWidget(m_ui->dataCacheWidget);
    connectCacheWidget(m_ui->instructionCacheWidget);

    // Make cache configuration changes emit processor reset requests
    connect(m_ui->dataCacheWidget, &CacheWidget::configurationChanged, [=] { emit m_parent->reqProcessorReset(); });
    connect(m_ui->instructionCacheWidget, &CacheWidget::configurationChanged,
            [=] { emit m_parent->reqProcessorReset(); });
}

void CacheTabWidget::connectCacheWidget(CacheWidget* w) {
    connect(w, &CacheWidget::cacheAddressSelected, this, &CacheTabWidget::focusAddressChanged);
}

CacheTabWidget::~CacheTabWidget() {
    delete m_ui;
}

}  // namespace Ripes
