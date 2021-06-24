#include "cachetabwidget.h"
#include "ui_cachetabwidget.h"

#include "memorytab.h"
#include "memoryviewerwidget.h"
#include "ripessettings.h"

#include <QTabBar>
#include <QWheelEvent>

namespace Ripes {

class ScrollEventFilter : public QObject {
public:
    ScrollEventFilter(QObject* parent) : QObject(parent) {}

    bool eventFilter(QObject*, QEvent* event) override {
        if (auto* wheelEvent = dynamic_cast<QWheelEvent*>(event)) {
            Q_UNUSED(wheelEvent);
            return true;
        }
        return false;
    }
};

void CacheTabWidget::flipTabs() {
    m_ui->tabWidget->setCurrentIndex(1);
    m_ui->tabWidget->setCurrentIndex(0);
}

CacheTabWidget::CacheTabWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::CacheTabWidget) {
    m_ui->setupUi(this);

    m_l1dShim = std::make_unique<L1CacheShim>(L1CacheShim::CacheType::DataCache, this);
    m_l1iShim = std::make_unique<L1CacheShim>(L1CacheShim::CacheType::InstrCache, this);

    m_l1dShim->setNextLevelCache(m_ui->dataCacheWidget->getCacheSim());
    m_l1iShim->setNextLevelCache(m_ui->instructionCacheWidget->getCacheSim());

#ifdef N_CACHES_ENABLED
    m_addTabIdx = m_ui->tabWidget->addTab(new QLabel("Placeholder"), QIcon((":/icons/plus.svg")), QString());
#endif

    connect(m_ui->tabWidget, &QTabWidget::currentChanged, this, &CacheTabWidget::handleTabIndexChanged);
    connect(m_ui->tabWidget, &QTabWidget::tabCloseRequested, this, &CacheTabWidget::handleTabCloseRequest);

    m_ui->tabWidget->setTabsClosable(true);

    if (auto* tabButton = m_ui->tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)) {
        // Store default close button size for future use
        m_defaultTabButtonSize = tabButton->size();

        // Disable closing for L1 caches and add tab
        tabButton->resize(0, 0);
        if (auto* tabButton2 = m_ui->tabWidget->tabBar()->tabButton(1, QTabBar::RightSide)) {
            tabButton2->resize(0, 0);
        }
    }

#ifdef N_CACHES_ENABLED
    if (auto* addTabButton = m_ui->tabWidget->tabBar()->tabButton(m_addTabIdx, QTabBar::RightSide)) {
        addTabButton->resize(0, 0);
    }
#endif

    // Filter out scroll events to avoid mouse scrolling creating a bazillion new caches
    m_ui->tabWidget->tabBar()->installEventFilter(new ScrollEventFilter(this));
}

void CacheTabWidget::handleTabCloseRequest(int index) {
    // Only the last-level cache should be closeable
    Q_ASSERT(index == m_addTabIdx - 1 && index > InstrCache);
    const int newIndex = index - 1;
    m_ui->tabWidget->setCurrentIndex(newIndex);
    m_ui->tabWidget->removeTab(index);
    m_addTabIdx = m_ui->tabWidget->count() - 1;
    if (newIndex > InstrCache) {
        m_ui->tabWidget->tabBar()->tabButton(newIndex, QTabBar::RightSide)->resize(m_defaultTabButtonSize);
    }
    m_nextCacheLevel--;
    m_ui->tabWidget->setTabText(m_addTabIdx, QString());
}

void CacheTabWidget::handleTabIndexChanged(int index) {
    if (index == m_addTabIdx) {
        // Add new level of cache
        auto* cw = new CacheWidget(this);
        m_ui->tabWidget->insertTab(m_addTabIdx, cw, QString("L%1 Cache").arg(m_nextCacheLevel));
        m_nextCacheLevel++;

        // The new cache is the deleteable cache, the one below it is thus no longer deleteable
        m_ui->tabWidget->tabBar()->tabButton(m_addTabIdx, QTabBar::RightSide)->resize(m_defaultTabButtonSize);
        m_ui->tabWidget->tabBar()->tabButton(m_addTabIdx - 1, QTabBar::RightSide)->resize(0, 0);
        m_addTabIdx = m_ui->tabWidget->count() - 1;
        m_ui->tabWidget->setCurrentIndex(index);
    }

    // Locate cacheWidget for the current index
    for (const auto& ch : m_ui->tabWidget->widget(index)->children()) {
        auto* cw = dynamic_cast<CacheWidget*>(ch);
        if (cw) {
            emit cacheFocusChanged(cw);
            break;
        }
    }
}

CacheTabWidget::~CacheTabWidget() {
    delete m_ui;
}

}  // namespace Ripes
