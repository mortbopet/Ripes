#include "memoryviewerwidget.h"
#include "ui_memoryviewerwidget.h"

#include <QTableView>

#include "gotocombobox.h"
#include "memorymodel.h"

MemoryViewerWidget::MemoryViewerWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::MemoryViewerWidget) {
    m_ui->setupUi(this);
    updateModel();
}

MemoryViewerWidget::~MemoryViewerWidget() {
    delete m_ui;
}

void MemoryViewerWidget::setCentralAddress(uint32_t address) {
    m_memoryModel->setCentralAddress(address);
}

void MemoryViewerWidget::updateView() {
    m_memoryModel->processorWasClocked();
}

void MemoryViewerWidget::updateModel() {
    auto* oldModel = m_memoryModel;

    m_memoryModel = new MemoryModel(this);
    m_ui->memoryView->setModel(m_memoryModel);
    m_ui->radixSelector->setRadix(m_memoryModel->getRadix());
    connect(m_ui->radixSelector, &RadixSelectorWidget::radixChanged, m_memoryModel, &MemoryModel::setRadix);

    // Connect scroll events on the view to modify the center address of the model
    connect(m_ui->memoryView, &MemoryView::scrolled, [=](bool dir) {
        if (dir) {
            m_memoryModel->offsetCentralAddress(1);
        } else {
            m_memoryModel->offsetCentralAddress(-1);
        }
    });

    m_memoryModel->setRowsVisible(1);
    connect(m_ui->memoryView, &MemoryView::resized, [=] {
        const auto rows = m_ui->memoryView->height() / m_ui->memoryView->rowHeight(0);
        m_memoryModel->setRowsVisible(rows);
    });
    connect(m_ui->gotoCombobox, &GoToComboBox::jumpToAddress, m_memoryModel, &MemoryModel::setCentralAddress);

    m_ui->memoryView->horizontalHeader()->setSectionResizeMode(MemoryModel::Address, QHeaderView::Stretch);

    if (oldModel) {
        delete oldModel;
    }
}
