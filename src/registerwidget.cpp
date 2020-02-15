#include "registerwidget.h"
#include "ui_registerwidget.h"

#include <QScrollBar>

#include "radixselectorwidget.h"

namespace Ripes {

RegisterWidget::RegisterWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::RegisterWidget) {
    m_ui->setupUi(this);
}

RegisterWidget::~RegisterWidget() {
    delete m_ui;
}

void RegisterWidget::updateModel() {
    auto* oldModel = m_registerModel;

    m_registerModel = new RegisterModel(this);
    m_ui->registerView->setModel(m_registerModel);
    m_ui->radixSelector->setRadix(m_registerModel->getRadix());
    connect(m_registerModel, &RegisterModel::registerChanged, this, &RegisterWidget::setRegisterviewCenterIndex);
    connect(m_ui->radixSelector, &RadixSelectorWidget::radixChanged, m_registerModel, &RegisterModel::setRadix);

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

void RegisterWidget::setRegisterviewCenterIndex(int index) {
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
}  // namespace Ripes
