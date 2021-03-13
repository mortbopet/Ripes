#include "iotab.h"
#include "ui_iotab.h"

#include <QDockWidget>
#include <QGraphicsItem>
#include <QMdiSubWindow>
#include <QToolBar>

#include "processorhandler.h"

#include "ioperipheraltab.h"

namespace Ripes {

IOTab::IOTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::IOTab) {
    m_ui->setupUi(this);

    m_ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_ui->peripheralsTab->clear();

    // Toolbar actions
    m_tileAction = new QAction(this);
    m_tileAction->setIcon(QIcon(":/icons/tile.svg"));
    m_tileAction->setText("Tile I/O windows");
    connect(m_tileAction, &QAction::triggered, this, &IOTab::tile);
    m_toolbar->addAction(m_tileAction);

    // Setup peripheral selection window
    m_ui->peripheralsTable->setRowCount(IOTypeTitles.size());
    m_ui->peripheralsTable->setColumnCount(1);
    m_ui->peripheralsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_ui->peripheralsTable->setHorizontalHeaderLabels({"Peripherals"});
    m_ui->peripheralsTable->verticalHeader()->hide();
    int row = 0;
    for (const auto& it : IOTypeTitles) {
        QTableWidgetItem* periphItem = new QTableWidgetItem(it.second);
        periphItem->setData(Qt::UserRole, QVariant::fromValue(it.first));
        periphItem->setToolTip("Double-click to create");
        periphItem->setFlags(periphItem->flags() & ~Qt::ItemIsEditable);
        m_ui->peripheralsTable->setItem(row++, 0, periphItem);
    }

    connect(m_ui->peripheralsTable, &QTableWidget::itemDoubleClicked,
            [this](QTableWidgetItem* item) { this->createPeripheral(item->data(Qt::UserRole).value<IOType>()); });

    // Setup MDI area
    connect(m_ui->mdiArea, &QMdiArea::subWindowActivated, [this](QMdiSubWindow* w) {
        if (w == nullptr) {
            setPeripheralTabActive(nullptr);
        } else {
            // MDI window -> QMainwindow -> QDockWidget -> IOBase widget... Whew!
            auto* w1 = w->widget();
            auto* w2 = w1->findChildren<QDockWidget*>().at(0);
            auto* w3 = w2->widget();
            auto* peripheral = dynamic_cast<IOBase*>(w3);
            Q_ASSERT(peripheral != nullptr);
            this->setPeripheralTabActive(peripheral);
        }
    });

    connect(m_ui->peripheralsTab, &QTabWidget::currentChanged, this, &IOTab::setPeripheralMDIWindowActive);
}

/**
 * Dummy function, should be moved to a separate memory map manager
 */
uint32_t nextAddress() {
    return 0x10000;
}

void IOTab::createPeripheral(IOType type) {
    auto* peripheral = m_iomanager.createPeripheral(type);
    // Create tab for peripheral
    auto* peripheralTab = new IOPeripheralTab(this, peripheral);
    m_ui->peripheralsTab->addTab(peripheralTab, peripheral->name());
    m_ioTabs[peripheral] = peripheralTab;

    // It seems excessive to create a QMainWindow for each peripheral but it seems like the only way to mix MDI
    // behaviour + dockable widgets that are able to pop out to a separate window
    auto* mw = new QMainWindow(this);
    m_ui->dockArea->addWidget(mw);  // Shouldn't be needed, but MDI windows aren't created without this?
    auto* dw = new QDockWidget(IOTypeTitles.at(type));
    dw->setFeatures(dw->features() & ~QDockWidget::DockWidgetClosable);
    dw->setWidget(peripheral);
    dw->setAllowedAreas(Qt::AllDockWidgetAreas);
    mw->addDockWidget(Qt::TopDockWidgetArea, dw);
    auto* mdiw = m_ui->mdiArea->addSubWindow(mw);

    m_subWindows[peripheralTab] = mdiw;

    // Signals

    // Connect to the destruction of the peripheral to trigger required changes on peripheral deletion. The peripheral
    // is implicitly destructed when the MDIWindow is closed.
    connect(peripheral, &IOBase::destroyed, this, &IOTab::removePeripheral);
}

void IOTab::setPeripheralTabActive(IOBase* peripheral) {
    if (peripheral == nullptr) {
        return;
    }
    m_ui->peripheralsTab->setCurrentWidget(m_ioTabs.at(peripheral));
}

void IOTab::setPeripheralMDIWindowActive(int tabIndex) {
    if (tabIndex == -1) {
        // No more peripherals
        return;
    }
    auto it = m_subWindows.find(m_ui->peripheralsTab->widget(tabIndex));
    if (it == m_subWindows.end()) {
        // We are currently creating the peripheral
        return;
    }

    m_ui->mdiArea->setActiveSubWindow(it->second);
}

void IOTab::removePeripheral(QObject* peripheral) {
    auto* tab = m_ioTabs.at(peripheral);
    Q_ASSERT(m_subWindows.count(tab) != 0);
    m_subWindows.erase(tab);
    m_ioTabs.at(peripheral)->deleteLater();
    m_ioTabs.erase(peripheral);
}

void IOTab::tile() {
    Q_ASSERT(false);
}

IOTab::~IOTab() {
    delete m_ui;
}
}  // namespace Ripes
