#include "cachetab.h"
#include "ui_cachetab.h"

#include "cachesim/cacheview.h"
#include "cachesim/cachewidget.h"

namespace Ripes {

CacheTab::CacheTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::CacheTab) {
    m_ui->setupUi(this);

    connect(m_ui->cacheView, &CacheView::cacheAddressSelected, this, &CacheTab::focusAddressChanged);

    connect(m_ui->cacheTabWidget, &CacheTabWidget::cacheFocusChanged, [=](CacheWidget* widget) {
        m_ui->cacheView->setScene(widget->getScene());
        m_ui->cacheView->fitScene();
    });

    // CacheTabWidget has a tendency to expand, but we'd like to minimize its size. Stretch the CacheView.
    m_ui->splitter->setStretchFactor(1, 10);
}

CacheTab::~CacheTab() {
    delete m_ui;
}

}  // namespace Ripes
