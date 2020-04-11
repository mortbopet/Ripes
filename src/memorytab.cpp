#include "memorytab.h"
#include "ui_memorytab.h"

#include "cachesim.h"
#include "cachegraphic.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QToolBar>

#include <QTimer>

namespace Ripes {

MemoryTab::MemoryTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::MemoryTab) {
    m_ui->setupUi(this);

    m_ui->memoryViewerWidget->updateModel();
    m_ui->memoryViewerWidget->updateView();

    auto* scene = new QGraphicsScene(this);
    auto* cacheSim = new CacheSim(this);
    m_ui->cacheConfig->setCache(cacheSim);

    connect(cacheSim, &CacheSim::hitRateChanged, m_ui->cacheConfig, &CacheConfigWidget::setHitRate);
    auto* cacheGraphic = new CacheGraphic(*cacheSim);
    m_ui->cacheView->setScene(scene);
    scene->addItem(cacheGraphic);

    auto* accessTimer = new QTimer(this);
    accessTimer->setInterval(100);
    connect(accessTimer, &QTimer::timeout, [=] {
        static unsigned address = 0x0;
        cacheSim->read(address);
        address += 4;
        if (address > 128) {
            address = 0;
        }
    });

    accessTimer->start();
}

void MemoryTab::update() {
    m_ui->memoryViewerWidget->updateView();
}

MemoryTab::~MemoryTab() {
    delete m_ui;
}
}  // namespace Ripes
