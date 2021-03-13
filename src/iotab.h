#pragma once

#include "defines.h"

#include <QAction>
#include <QMdiSubWindow>
#include <QWidget>

#include <unordered_map>

#include "memorymodel.h"
#include "processorhandler.h"
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

private slots:
    void removePeripheral(QObject* peripheral);

    /** The following to slots ensures synchronization between the current MDI window and peripheral tab */
    void setPeripheralTabActive(IOBase* peripheral);
    void setPeripheralMDIWindowActive(int tabIndex);

private:
    void createPeripheral(IOType type);

    void tile();
    Ui::IOTab* m_ui = nullptr;
    QAction* m_tileAction = nullptr;

    /**
     * @brief m_ioTabs
     * Pointer from the (QObject*) peripheral to its corresponding IO tab. We maintain a QObject* here rather than the
     * IOBase* given that the association is used during emission of the QObject::destroyed signal. In this case, the
     * IOBase* subclass has already been destroyed.
     */
    std::unordered_map<QObject*, QWidget*> m_ioTabs;
    /**
     * @brief m_ioTabs
     * Pointer from the tab associated with a peripheral to its corresponding MDI subwindow.
     */
    std::unordered_map<QWidget*, QMdiSubWindow*> m_subWindows;
    IOManager m_iomanager;
};
}  // namespace Ripes
