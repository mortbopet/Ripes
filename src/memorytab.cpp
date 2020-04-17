#include "memorytab.h"
#include "ui_memorytab.h"

#include <QGraphicsItem>
#include <QToolBar>

#include "cachesim/cachesim.h"

namespace Ripes {

MemoryTab::MemoryTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::MemoryTab) {
    m_ui->setupUi(this);

    m_ui->memoryViewerWidget->updateModel();
    m_ui->memoryViewerWidget->updateView();

    m_ui->dataCache->setType(CacheSim::CacheType::DataCache);
    m_ui->instructionCache->setType(CacheSim::CacheType::InstrCache);

    // Make selection changes in the cache trigger the memory viewer to set its central address to the selected address
    connect(m_ui->dataCache, &CacheWidget::cacheAddressSelected, m_ui->memoryViewerWidget,
            &MemoryViewerWidget::setCentralAddress);
    connect(m_ui->instructionCache, &CacheWidget::cacheAddressSelected, m_ui->memoryViewerWidget,
            &MemoryViewerWidget::setCentralAddress);
}

void MemoryTab::update() {
    m_ui->memoryViewerWidget->updateView();
}

MemoryTab::~MemoryTab() {
    delete m_ui;
}
}  // namespace Ripes
