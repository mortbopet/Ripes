#include "memorytab.h"
#include "ui_memorytab.h"

#include <QGraphicsItem>
#include <QPushButton>
#include <QToolBar>

#include "cachesim/cachesim.h"
#include "io/iomanager.h"
#include "io/memorymapmodel.h"
#include "processorhandler.h"

namespace Ripes {

MemoryTab::MemoryTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::MemoryTab) {
    m_ui->setupUi(this);

    m_ui->memoryViewerWidget->updateModel();
    m_ui->memoryViewerWidget->updateView();

    // During processor running, it should not be possible to interact with the memory viewer or cache widgets
    connect(ProcessorHandler::get(), &ProcessorHandler::runStarted, this, [=] { setEnabled(false); });
    connect(ProcessorHandler::get(), &ProcessorHandler::runFinished, this, [=] { setEnabled(true); });

    m_ui->memoryMapView->setModel(new MemoryMapModel(&IOManager::get(), this));
    m_ui->memoryMapView->horizontalHeader()->setSectionResizeMode(MemoryMapModel::Name, QHeaderView::ResizeToContents);
    m_ui->memoryMapView->horizontalHeader()->setSectionResizeMode(MemoryMapModel::AddressRange, QHeaderView::Stretch);
    m_ui->memoryMapView->horizontalHeader()->setSectionResizeMode(MemoryMapModel::Size, QHeaderView::ResizeToContents);
    m_ui->splitter->setStretchFactor(0, 2);
    m_ui->splitter->setStretchFactor(1, 1);

    connect(ProcessorHandler::get(), &ProcessorHandler::procStateChangedNonRun, m_ui->memoryViewerWidget,
            [=] { m_ui->memoryViewerWidget->updateView(); });
}

MemoryTab::~MemoryTab() {
    delete m_ui;
}

void MemoryTab::setCentralAddress(unsigned int address) {
    m_ui->memoryViewerWidget->setCentralAddress(address);
}

}  // namespace Ripes
