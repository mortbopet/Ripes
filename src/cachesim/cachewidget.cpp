#include "cachewidget.h"
#include "ui_cachewidget.h"

#include <QGraphicsScene>
#include <QGraphicsView>

#include "cachegraphic.h"

namespace Ripes {

CacheWidget::CacheWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheWidget) {
    m_ui->setupUi(this);

    auto* scene = new QGraphicsScene(this);
    m_cacheSim = new CacheSim(this);
    m_ui->cacheConfig->setCache(m_cacheSim);

    auto* cacheGraphic = new CacheGraphic(*m_cacheSim);
    m_ui->cacheView->setScene(scene);
    scene->addItem(cacheGraphic);

    connect(m_ui->cacheView, &CacheView::cacheAddressSelected,
            [=](uint32_t address) { emit cacheAddressSelected(address); });

    connect(m_cacheSim, &CacheSim::configurationChanged, [=] { emit configurationChanged(); });
}

void CacheWidget::setType(CacheSim::CacheType type) {
    m_cacheSim->setType(type);
}

CacheWidget::~CacheWidget() {
    delete m_ui;
}

}  // namespace Ripes
