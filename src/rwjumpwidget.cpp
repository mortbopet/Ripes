#include "rwjumpwidget.h"
#include "ui_rwjumpwidget.h"

#include <QHeaderView>

RWJumpWidget::RWJumpWidget(QWidget* parent) : QWidget(parent), ui(new Ui::RWJumpWidget) {
    ui->setupUi(this);
}

void RWJumpWidget::init() {
    m_model = new RWJumpModel();
    ui->view->setModel(m_model);
    ui->view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->view->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->view->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->view->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void RWJumpWidget::updateModel() {
    m_model->update();
}

RWJumpWidget::~RWJumpWidget() {
    delete ui;
}

void RWJumpWidget::on_jump_clicked() {
    // Get selected index
    auto selecter = ui->view->selectionModel();
    if (selecter->hasSelection()) {
        auto indexes = selecter->selectedIndexes();
        emit jumpToAdress(m_model->data(indexes[0], Qt::UserRole).value<uint32_t>());
    }
}
