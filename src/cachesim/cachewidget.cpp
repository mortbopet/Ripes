#include "cachewidget.h"
#include "ui_cachewidget.h"

#include <QGraphicsScene>
#include <QGraphicsView>

#include "cachegraphic.h"
#include "cacheview.h"
#include "ripessettings.h"

namespace Ripes {

CacheWidget::CacheWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheWidget) {
    m_ui->setupUi(this);

    m_scene = std::make_unique<QGraphicsScene>(this);
    m_cacheSim = std::make_shared<CacheSim>(this);
    m_ui->cacheConfig->setCache(m_cacheSim);
    m_ui->cachePlot->setCache(m_cacheSim);

    auto* cacheGraphic = new CacheGraphic(*m_cacheSim);
    m_scene->addItem(cacheGraphic);
    connect(m_cacheSim.get(), &CacheSim::configurationChanged, this, [=] {
        RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET)->trigger();

        auto cacheViews = m_scene->views();
        if (cacheViews.size() > 0) {
            static_cast<CacheView*>(cacheViews.at(0))->fitScene();
        }
    });
}

void CacheWidget::setNextLevelCache(const std::shared_ptr<CacheSim>& cache) {
    m_cacheSim.get()->setNextLevelCache(cache);
}

CacheWidget::~CacheWidget() {
    delete m_ui;
}

}  // namespace Ripes
