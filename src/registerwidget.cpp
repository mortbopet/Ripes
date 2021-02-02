#include "registerwidget.h"
#include "ui_registerwidget.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QTableView>

#include "processorhandler.h"
#include "radixselectorwidget.h"

namespace Ripes {

RegisterWidget::RegisterWidget(const RegisterFileType regFileID, QWidget* parent)
    : QWidget(parent), m_ui(new Ui::RegisterWidget), m_regFileID(regFileID) {
    m_ui->setupUi(this);
}

RegisterWidget::~RegisterWidget() {
    delete m_ui;
}

void RegisterWidget::initialize() {
    if (m_model) {
        m_model->deleteLater();
    }

    m_model = new RegisterModel(m_regFileID, this);
    m_ui->registerView->setModel(m_model);

    m_ui->registerView->horizontalHeader()->setSectionResizeMode(RegisterModel::Name, QHeaderView::ResizeToContents);
    m_ui->registerView->horizontalHeader()->setSectionResizeMode(RegisterModel::Alias, QHeaderView::ResizeToContents);
    m_ui->registerView->horizontalHeader()->setSectionResizeMode(RegisterModel::Value, QHeaderView::Stretch);

    m_ui->radixSelector->setRadix(m_model->getRadix());
    connect(m_model, &RegisterModel::registerChanged, this, &RegisterWidget::setRegisterviewCenterIndex);
    connect(m_ui->radixSelector, &RadixSelectorWidget::radixChanged, m_model, &RegisterModel::setRadix);
}

void RegisterWidget::updateView() {
    m_model->processorWasClocked();
}

void RegisterWidget::setRegisterviewCenterIndex(int index) {
    const auto view = m_ui->registerView;
    const auto rect = view->rect();
    int indexTop = view->indexAt(rect.topLeft()).row();
    int indexBot = view->indexAt(rect.bottomLeft()).row();
    indexBot = indexBot < 0 ? m_model->rowCount() : indexBot;

    const int nItemsVisible = indexBot - indexTop;

    // move scrollbar if if is not visible
    if (index <= indexTop || index >= indexBot) {
        auto scrollbar = view->verticalScrollBar();
        scrollbar->setValue(index - nItemsVisible / 2);
    }
}
}  // namespace Ripes
