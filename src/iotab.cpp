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

    m_ui->mdiArea->addSubWindow(new IOLedMatrix(this, 0x10000, 25));
}

IOTab::~IOTab() {
    delete m_ui;
}
}  // namespace Ripes
