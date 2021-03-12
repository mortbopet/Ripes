#include "iotab.h"
#include "ui_iotab.h"

#include <QDockWidget>
#include <QGraphicsItem>
#include <QMdiSubWindow>
#include <QToolBar>

#include "processorhandler.h"

namespace Ripes {

IOTab::IOTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::IOTab) {
    m_ui->setupUi(this);

    m_ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

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
            setPeripheralActive(nullptr);
        } else {
            // MDI window -> QMainwindow -> QDockWidget -> IOBase widget... Whew!
            auto* w1 = w->widget();
            auto* w2 = w1->findChildren<QDockWidget*>().at(0);
            auto* w3 = w2->widget();
            auto* peripheral = dynamic_cast<IOBase*>(w3);
            Q_ASSERT(peripheral != nullptr);
            this->setPeripheralActive(peripheral);
        }
    });
}

/**
 * Dummy function, should be moved to a separate memory map manager
 */
uint32_t nextAddress() {
    return 0x10000;
}

void IOTab::createPeripheral(IOType type) {
    // It seems excessive to create a QMainWindow for each peripheral but it seems like the only way to mix MDI
    // behaviour + dockable widgets that are able to pop out to a separate window
    auto* mw = new QMainWindow(this);
    m_ui->dockArea->addWidget(mw);  // Shouldn't be needed, but MDI windows aren't created without this?
    auto* dw = new QDockWidget(IOTypeTitles.at(type));
    dw->setWidget(IOFactories.at(type)(this, nextAddress()));
    dw->setAllowedAreas(Qt::AllDockWidgetAreas);
    mw->addDockWidget(Qt::TopDockWidgetArea, dw);
    auto* sw = m_ui->mdiArea->addSubWindow(mw);

    connect(sw, &QObject::destroyed, this, &IOTab::removePeripheral);
}

void IOTab::setPeripheralActive(IOBase* peripheral) {
    if (peripheral == nullptr) {
        // No more widgets left, figure out what to do with the tab...
    } else {
        // ...
    }
}

void IOTab::removePeripheral(QObject* peripheral) {}

void IOTab::tile() {
    Q_ASSERT(false);
}

IOTab::~IOTab() {
    delete m_ui;
}
}  // namespace Ripes
