#include "registerwidget.h"
#include "ui_registerwidget.h"

RegisterWidget::RegisterWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::RegisterWidget) {
    m_ui->setupUi(this);
    setupRadixComboBox();
}

RegisterWidget::~RegisterWidget() {
    delete m_ui;
}

void RegisterWidget::setHandler(ProcessorHandler* handler) {
    m_handler = handler;
    updateModel();
}

void RegisterWidget::setupRadixComboBox() {
    for (const auto& rt : s_radixName) {
        m_ui->displayMode->addItem(rt.second, QVariant::fromValue(rt.first));
    }

    connect(m_ui->displayMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        Radix r = qvariant_cast<Radix>(m_ui->displayMode->itemData(index));
        m_registerModel->setRadix(r);
    });
}

void RegisterWidget::updateRadixComboBoxIndex() {
    // Synchronize the index of the radix selection combo box with the radix currently set in the model
    for (int i = 0; i < m_ui->displayMode->count(); i++) {
        const Radix itemRadix = qvariant_cast<Radix>(m_ui->displayMode->itemData(i));
        if (m_registerModel->getRadix() == itemRadix) {
            m_ui->displayMode->setCurrentIndex(i);
            return;
        }
    }
}

void RegisterWidget::updateModel() {
    auto* oldModel = m_registerModel;

    m_registerModel = new RegisterModel(*m_handler, this);
    m_ui->registerView->setModel(m_registerModel);
    updateRadixComboBoxIndex();

    m_ui->registerView->horizontalHeader()->setSectionResizeMode(RegisterModel::Name, QHeaderView::ResizeToContents);
    m_ui->registerView->horizontalHeader()->setSectionResizeMode(RegisterModel::Alias, QHeaderView::ResizeToContents);
    m_ui->registerView->horizontalHeader()->setSectionResizeMode(RegisterModel::Value, QHeaderView::Stretch);

    if (oldModel) {
        delete oldModel;
    }
}

void RegisterWidget::updateView() {
    m_registerModel->processorWasClocked();
}
