#include "cachetab.h"
#include "ui_cachetab.h"

#include "cachesim/cacheview.h"
#include "cachesim/cachewidget.h"

#include "processorhandler.h"

namespace Ripes {

CacheTab::CacheTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::CacheTab) {
    m_ui->setupUi(this);

    // During processor running, it should not be possible to interact with the cache tab
    connect(ProcessorHandler::get(), &ProcessorHandler::runStarted, this, [=] { setEnabled(false); });
    connect(ProcessorHandler::get(), &ProcessorHandler::runFinished, this, [=] { setEnabled(true); });

    connect(m_ui->cacheView, &CacheView::cacheAddressSelected, this, &CacheTab::focusAddressChanged);

    connect(m_ui->cacheTabWidget, &CacheTabWidget::cacheFocusChanged, this, [=](CacheWidget* widget) {
        m_ui->cacheView->setScene(widget->getScene());
        m_ui->cacheView->fitScene();
    });

    // CacheTabWidget has a tendency to expand, but we'd like to minimize its size. Stretch the CacheView.
    m_ui->splitter->setStretchFactor(0, 0);
    m_ui->splitter->setStretchFactor(1, 10);
    m_ui->splitter->setSizes({1, 10000});
}

void CacheTab::tabVisibilityChanged(bool visible) {
    /**
     * A hack to ensure that the cache view is resized to screen size _after_ Qt has calculated widget sizes when the
     * cache tab is first shown.
     */
    if (!m_initialized && visible) {
        QTimer::singleShot(100, m_ui->cacheTabWidget, [&] { m_ui->cacheTabWidget->flipTabs(); });
        m_initialized = visible;
    }
}

CacheTab::~CacheTab() {
    delete m_ui;
}

}  // namespace Ripes
