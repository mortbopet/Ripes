#include "cachewidget.h"
#include "ui_cachewidget.h"

#include <QGraphicsScene>
#include <QGraphicsView>

#include "cachegraphic.h"
#include "cachesim.h"

namespace Ripes {

CacheWidget::CacheWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheWidget) {
    m_ui->setupUi(this);

    auto* scene = new QGraphicsScene(this);
    auto* cacheSim = new CacheSim(this);
    m_ui->cacheConfig->setCache(cacheSim);

    auto* cacheGraphic = new CacheGraphic(*cacheSim);
    m_ui->cacheView->setScene(scene);
    scene->addItem(cacheGraphic);
}

CacheWidget::~CacheWidget() {
    delete m_ui;
}

}  // namespace Ripes
