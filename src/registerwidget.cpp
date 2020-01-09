#include "registerwidget.h"
#include "ui_registerwidget.h"

#include <QScrollBar>

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
    connect(m_registerModel, &RegisterModel::registerChanged, this, &RegisterWidget::setRegisterviewCenterIndex);

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

void RegisterWidget::setRegisterviewCenterIndex(unsigned index) {
    const auto view = m_ui->registerView;
    const auto rect = view->rect();
    int indexTop = view->indexAt(rect.topLeft()).row();
    int indexBot = view->indexAt(rect.bottomLeft()).row();
    indexBot = indexBot < 0 ? m_registerModel->rowCount() : indexBot;

    const int nItemsVisible = indexBot - indexTop;

    // move scrollbar if if is not visible
    if (index <= indexTop || index >= indexBot) {
        auto scrollbar = view->verticalScrollBar();
        scrollbar->setValue(index - nItemsVisible / 2);
    }
}
