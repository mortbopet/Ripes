#include "memorytab.h"
#include "ui_memorytab.h"

#include <QToolBar>

#include "pipeline.h"

MemoryTab::MemoryTab(ProcessorHandler& handler, QToolBar* toolbar, QWidget* parent)
    : RipesTab(toolbar, parent), m_handler(handler), m_ui(new Ui::MemoryTab) {
    m_ui->setupUi(this);

    m_ui->memoryViewerWidget->setHandler(&m_handler);
    m_ui->memoryViewerWidget->updateModel();
    m_ui->memoryViewerWidget->updateView();

    m_ui->rwjumpwidget->init();
    connect(m_ui->rwjumpwidget, &RWJumpWidget::jumpToAdress, m_ui->memoryViewerWidget,
            &MemoryViewerWidget::setCentralAddress);
}

void MemoryTab::update() {
    m_ui->rwjumpwidget->updateModel();
    m_ui->memoryViewerWidget->updateView();
}

MemoryTab::~MemoryTab() {
    delete m_ui;
}
