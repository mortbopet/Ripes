#include "memorytab.h"
#include "ui_memorytab.h"

#include <QInputDialog>
#include <QWheelEvent>
#include <algorithm>

#include "pipeline.h"

MemoryTab::MemoryTab(QWidget* parent) : QWidget(parent), m_ui(new Ui::MemoryTab) {
    m_ui->setupUi(this);

    // Add display types to display comboboxes
    for (const auto& type : displayTypes.keys()) {
        m_ui->memorydisplaytype->insertItem(0, type, displayTypes[type]);
    }

    m_ui->memorydisplaytype->setCurrentIndex(displayTypeN::Hex);
}

void MemoryTab::initMemoryTab() {
    m_memoryPtr = Pipeline::getPipeline()->getRuntimeMemoryPtr();
    m_regPtr = Pipeline::getPipeline()->getRegPtr();
    initializeMemoryView();
    m_ui->registerContainer->setRegPtr(m_regPtr);
    m_ui->registerContainer->init();
    m_ui->rwjumpwidget->init();
    connect(m_ui->rwjumpwidget, &RWJumpWidget::jumpToAdress, this, &MemoryTab::jumpToAdress);
}

void MemoryTab::saveAddress() {
    // Saves the current centerAddress of the model to be available as a jump-to
    // address in the view

    QInputDialog dialog;
    dialog.setWindowIcon(QIcon(":/icons/logo.svg"));
    dialog.setLabelText(
        QString("Please enter a label for address: %1")
            .arg(QString("0x%1").arg(QString().setNum(m_model->getCentralAddress(), 16).rightJustified(8, '0'))));
    if (dialog.exec() == QDialog::Accepted) {
        m_ui->gotoCombobox->addItem(dialog.textValue(), m_model->getCentralAddress());
    }
}

void MemoryTab::initializeMemoryView() {
    m_model = new MemoryModel(m_memoryPtr);
    m_delegate = new MemoryDisplayDelegate();
    m_ui->memoryView->setModel(m_model);
    m_ui->memoryView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_ui->memoryView->verticalHeader()->hide();

    // Only set delegate on byte columns
    m_ui->memoryView->setItemDelegateForColumn(1, m_delegate);
    m_ui->memoryView->setItemDelegateForColumn(2, m_delegate);
    m_ui->memoryView->setItemDelegateForColumn(3, m_delegate);
    m_ui->memoryView->setItemDelegateForColumn(4, m_delegate);

    // Memory display type
    connect(m_ui->memorydisplaytype, &QComboBox::currentTextChanged, m_delegate, [=] {
        m_delegate->setDisplayType(qvariant_cast<displayTypeN>(m_ui->memorydisplaytype->currentData()));
        m_ui->memoryView->viewport()->repaint();
    });
    m_delegate->setDisplayType(qvariant_cast<displayTypeN>(m_ui->memorydisplaytype->currentData()));

    // connect up/down buttons to adjust the central address of the model
    connect(m_ui->memoryUp, &QPushButton::clicked, [=] {
        m_model->offsetCentralAddress(4);
        m_ui->memoryView->viewport()->repaint();
    });
    connect(m_ui->memoryDown, &QPushButton::clicked, [=] {
        m_model->offsetCentralAddress(-4);
        m_ui->memoryView->viewport()->repaint();
    });

    // Connect scroll events directly to the model
    connect(m_ui->memoryView, &MemoryView::scrolled, [=](bool dir) {
        if (dir) {
            m_model->offsetCentralAddress(4);
        } else {
            m_model->offsetCentralAddress(-4);
        };
    });

    // Disable editing of memory - this is not implemented in the delegate
    m_ui->memoryView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Connect goto combobox

    connect(m_ui->gotoCombobox, &GoToComboBox::jumpToAddress, m_model, &MemoryModel::jumpToAddress);

    // Connect save address button
    connect(m_ui->save, &QPushButton::clicked, this, &MemoryTab::saveAddress);

    connect(m_model, &MemoryModel::dataChanged, m_ui->memoryView, &MemoryView::setVisibleRows);
}

void MemoryTab::update() {
    m_ui->registerContainer->update();
    m_model->updateModel();
    m_ui->rwjumpwidget->updateModel();
}

void MemoryTab::jumpToAdress(uint32_t address) {
    m_model->jumpToAddress(address);
}

MemoryTab::~MemoryTab() {
    delete m_ui;
}
