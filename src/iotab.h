#pragma once

#include <QAction>
#include <QMdiSubWindow>
#include <QWidget>

#include <unordered_map>

#include "memorymodel.h"
#include "ripestab.h"

#include "io/iomanager.h"
#include "io/ioregistry.h"

namespace Ripes {

namespace Ui {
class IOTab;
}

class IOTab : public RipesTab {
    Q_OBJECT

public:
    IOTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~IOTab() override;
    void removePeripheral(QObject* peripheral);

private slots:

    /** The following to slots ensures synchronization between the current MDI window and peripheral tab */
    void setPeripheralTabActive(Ripes::IOBase* peripheral);
    void setPeripheralMDIWindowActive(int tabIndex);

private:
    IOBase* createPeripheral(IOType type, int forcedID = -1);

    /**
     * @brief Peripheral serialization
     * To make peripherals persist between invocations of the program, we serialize the state of all instantiated
     * peripherals into a settings variable.
     * Each peripheral is uniquely identified by a key (see IOBase::serializedID()). For each peripheral, we then store
     * the values of its parameters (see IOParam).
     */
    void loadPeripheralState();
    void storePeripheralState();
    void updateIOSymbolFilePreview();

    void tile();
    Ui::IOTab* m_ui = nullptr;

    /**
     * @brief m_ioTabs
     * Pointer from the (QObject*) peripheral to its corresponding IO tab. We maintain a QObject* here rather than the
     * IOBase* given that the association is used during emission of the QObject::destroyed signal. In this case, the
     * IOBase* subclass has already been destroyed.
     */
    std::unordered_map<QObject*, QWidget*> m_periphToTab;

    /**
     * @brief m_ioTabs
     * Pointer from the tab associated with a peripheral to its corresponding MDI subwindow.
     */
    std::unordered_map<QWidget*, QMdiSubWindow*> m_subWindows;
};
}  // namespace Ripes
