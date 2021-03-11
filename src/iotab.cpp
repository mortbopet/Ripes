#include "iotab.h"
#include "ui_iotab.h"

#include <QDockWidget>
#include <QGraphicsItem>
#include <QToolBar>

#include "processorhandler.h"

// IO components
#include "io/ioledmatrix.h"

namespace Ripes {

IOTab::IOTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::IOTab) {
    m_ui->setupUi(this);

    m_ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    for (int i = 0; i < 10; i++) {
        // It seems excessive to create a QMainWindow for each peripheral but it seems like the only way to mix MDI
        // behaviour + dockable widgets that are able to pop out to a separate window
        auto* mw = new QMainWindow(this);
        m_ui->dockArea->addWidget(mw);
        auto* dw = new QDockWidget("LED Matrix");
        dw->setWidget(new IOLedMatrix(this, 0x10000, 25));
        dw->setAllowedAreas(Qt::AllDockWidgetAreas);
        mw->addDockWidget(Qt::TopDockWidgetArea, dw);
        m_ui->mdiArea->addSubWindow(mw);
    }

    // Toolbar actions
    m_tileAction = new QAction(this);
    m_tileAction->setIcon(QIcon(":/icons/tile.svg"));
    m_tileAction->setText("Tile I/O windows");
    connect(m_tileAction, &QAction::triggered, this, &IOTab::tile);
    m_toolbar->addAction(m_tileAction);
}

void IOTab::tile() {
    Q_ASSERT(false);
}

IOTab::~IOTab() {
    delete m_ui;
}
}  // namespace Ripes
