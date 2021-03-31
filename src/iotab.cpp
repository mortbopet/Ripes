#include "iotab.h"
#include "ui_iotab.h"

#include <QDockWidget>
#include <QGraphicsItem>
#include <QMdiSubWindow>
#include <QToolBar>

#include "io/memorymapmodel.h"
#include "processorhandler.h"
#include "ripessettings.h"

#include "ioperipheraltab.h"

#include "VSRTL/external/cereal/include/cereal/archives/json.hpp"
#include "VSRTL/external/cereal/include/cereal/types/map.hpp"
#include "VSRTL/external/cereal/include/cereal/types/vector.hpp"

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

    connect(&IOManager::get(), &IOManager::peripheralRemoved, QPointer(this), &IOTab::removePeripheral);

    connect(m_ui->peripheralsTab, &QTabWidget::currentChanged, this, &IOTab::setPeripheralMDIWindowActive);

    // Store peripheral state before exiting the program
    connect(RipesSettings::getObserver(RIPES_GLOBALSIGNAL_QUIT), &SettingObserver::modified, this,
            &IOTab::storePeripheralState);

    // Reload peripheral state from the last graceful exit of the program
    loadPeripheralState();
}

IOBase* IOTab::createPeripheral(IOType type, int forcedID) {
    auto* peripheral = IOManager::get().createPeripheral(type);
    if (forcedID != -1) {
        peripheral->setID(forcedID);
    }
    // Create tab for peripheral
    auto* peripheralTab = new IOPeripheralTab(this, peripheral);
    m_ui->peripheralsTab->addTab(peripheralTab, peripheral->name());
    m_periphToTab[peripheral] = peripheralTab;

    // It seems excessive to create a QMainWindow for each peripheral but it seems like the only way to mix MDI
    // behaviour + dockable widgets that are able to pop out to a separate window
    auto* mw = new QMainWindow(this);
    m_ui->dockArea->addWidget(mw);  // Shouldn't be needed, but MDI windows aren't created without this?
    auto* dw = new QDockWidget();
    dw->setFeatures(dw->features() & ~QDockWidget::DockWidgetClosable);
    dw->setWidget(peripheral);
    dw->setAllowedAreas(Qt::AllDockWidgetAreas);
    mw->addDockWidget(Qt::TopDockWidgetArea, dw);
    auto* mdiw = m_ui->mdiArea->addSubWindow(mw);
    mdiw->setWindowTitle(peripheral->name());
    peripheral->setFocus();

    m_subWindows[peripheralTab] = mdiw;
    return peripheral;
}

void IOTab::loadPeripheralState() {
    auto settings = RipesSettings::value(RIPES_SETTING_PERIPHERAL_SETTINGS).toString().toStdString();
    if (settings.empty()) {
        return;
    }

    std::istringstream in(settings);
    try {
        cereal::JSONInputArchive archive(in);

        // Following the serialization order of storePeripheralState(), we first load the peripheral types and unique
        // IDs, instantiate peripherals, and then load the peripheral settings.
        PeriphIDs ids;
        try {
            archive(cereal::make_nvp("periphIDs", ids));
            for (const auto& id : ids) {
                if (id.typeId < NPERIPHERALS) {
                    auto* periph = createPeripheral(static_cast<IOType>(id.typeId), id.uniqueId);
                }
            }

            for (const auto& periph : m_periphToTab) {
                IOBase* periphPtr = dynamic_cast<IOBase*>(periph.first);
                archive(cereal::make_nvp(periphPtr->serializedUniqueID(), *periphPtr));
            }
        } catch (cereal::Exception e) {
            /// @todo: build an error report
        }
    } catch (...) {
        // Could not load peripherals from settings...
        return;
    }
}

void IOTab::storePeripheralState() {
    std::ostringstream out;
    {
        cereal::JSONOutputArchive archive(out);

        // First, serialize the type of each peripheral, and then the peripheral itself
        PeriphIDs ids;
        for (const auto& periph : m_periphToTab) {
            IOBase* periphPtr = dynamic_cast<IOBase*>(periph.first);
            assert(periphPtr);
            ids.push_back({periphPtr->iotype(), periphPtr->id()});
        }

        try {
            archive(cereal::make_nvp("periphIDs", ids));
            for (const auto& periph : m_periphToTab) {
                IOBase* periphPtr = dynamic_cast<IOBase*>(periph.first);
                archive(cereal::make_nvp(periphPtr->serializedUniqueID(), *periphPtr));
            }
        } catch (cereal::Exception e) {
            /// @todo: build an error report
        }
    }
    RipesSettings::setValue(RIPES_SETTING_PERIPHERAL_SETTINGS, QString::fromStdString(out.str()));
}

void IOTab::setPeripheralTabActive(IOBase* peripheral) {
    if (peripheral == nullptr) {
        return;
    }
    m_ui->peripheralsTab->setCurrentWidget(m_periphToTab.at(peripheral));
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
    auto* tab = m_periphToTab.at(peripheral);
    Q_ASSERT(m_subWindows.count(tab) != 0);
    m_subWindows.erase(tab);
    m_periphToTab.at(peripheral)->deleteLater();
    m_periphToTab.erase(peripheral);
}

void IOTab::tile() {
    Q_ASSERT(false);
}

IOTab::~IOTab() {
    /* Because of the way that IOBase objects signal to the IOTab that they have been removed, we need to delete all
     * IOBase objects before deleting the IOTab itself. The default deletion mechanism is incorrect for this, given that
     * IOTab is first deleted, and then the underlying QObject is deleted (which deletes its children, being the IOBase
     * objects). We delete from the subwindows because they are the top-level parent of the IOBase objects.
     */

    // Copy subwindows collection, so we can safely iterate through it (m_subWindows is modified when deleting a
    // subwindow).
    auto subWindowsCopy = m_subWindows;
    for (const auto& subwindow : subWindowsCopy) {
        delete subwindow.second;
    }
    delete m_ui;
}
}  // namespace Ripes
