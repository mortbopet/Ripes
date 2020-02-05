#include "memorytab.h"
#include "ui_memorytab.h"

#include <QToolBar>

namespace Ripes {

MemoryTab::MemoryTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::MemoryTab) {
    m_ui->setupUi(this);

    m_ui->memoryViewerWidget->updateModel();
    m_ui->memoryViewerWidget->updateView();

    m_ui->splitter->setStretchFactor(1, 1);
}

void MemoryTab::update() {
    m_ui->memoryViewerWidget->updateView();
}

MemoryTab::~MemoryTab() {
    delete m_ui;
}
}  // namespace Ripes
