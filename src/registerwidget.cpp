#include "registerwidget.h"
#include "ui_registerwidget.h"

#include <QClipboard>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QScrollBar>
#include <QTableView>

#include "addressdialog.h"
#include "processorhandler.h"
#include "radixselectorwidget.h"
#include "registermodel.h"

namespace Ripes {

RegisterWidget::RegisterWidget(const RegisterFileType regFileID,
                               QWidget *parent)
    : QWidget(parent), m_ui(new Ui::RegisterWidget), m_regFileID(regFileID) {
  m_ui->setupUi(this);
}

RegisterWidget::~RegisterWidget() { delete m_ui; }

void RegisterWidget::initialize() {
  if (m_model) {
    m_model->deleteLater();
  }

  m_model = new RegisterModel(m_regFileID, this);
  m_ui->registerView->setModel(m_model);

  // Add custom right click menu to register view
  m_ui->registerView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ui->registerView, &QTableView::customContextMenuRequested, this,
          &RegisterWidget::showContextMenu);

  m_ui->registerView->horizontalHeader()->setSectionResizeMode(
      RegisterModel::Name, QHeaderView::ResizeToContents);
  m_ui->registerView->horizontalHeader()->setSectionResizeMode(
      RegisterModel::Alias, QHeaderView::ResizeToContents);
  m_ui->registerView->horizontalHeader()->setSectionResizeMode(
      RegisterModel::Value, QHeaderView::Stretch);

  m_ui->radixSelector->setRadix(m_model->getRadix());
  connect(m_model, &RegisterModel::registerChanged, this,
          &RegisterWidget::setRegisterviewCenterIndex);
  connect(m_ui->radixSelector, &RadixSelectorWidget::radixChanged, m_model,
          &RegisterModel::setRadix);
}

void RegisterWidget::updateView() { m_model->processorWasClocked(); }

void RegisterWidget::showContextMenu(const QPoint &pos) {
  const auto index = m_ui->registerView->indexAt(pos);
  if (!index.isValid())
    return;

  // Only allow context menu on the value column
  if (index.column() != RegisterModel::Value)
    return;

  QMenu menu;
  menu.addAction("Copy register value", [&]() {
    QApplication::clipboard()->setText(
        m_model->data(index, Qt::DisplayRole).toString());
  });

  // If the register is not read only, add "set register value" action.
  if (m_model->flags(index) & Qt::ItemIsEditable) {
    menu.addAction("Set register value", [&]() {
      QInputDialog dialog(this);
      dialog.setWindowTitle("Set register value");
      dialog.setLabelText("Register value");
      dialog.setTextValue(m_model->data(index, Qt::DisplayRole).toString());
      dialog.setOkButtonText("Set");
      dialog.setCancelButtonText("Cancel");
      dialog.setWindowFlags(dialog.windowFlags() &
                            ~Qt::WindowContextHelpButtonHint);
      if (dialog.exec() == QDialog::Accepted) {
        m_model->setData(index, dialog.textValue(), /*unused*/ 0);
      }
    });
  }

  auto goToAddressAction = QAction("Go to address");
  goToAddressAction.setToolTip("Make all memory viewers move to the address "
                               "contained in the selected register");
  connect(&goToAddressAction, &QAction::triggered, [&]() {
    const auto address = m_model->data(index, Qt::EditRole).toULongLong();
    ProcessorHandler::setMemoryFocusAddress(address);
  });
  menu.addAction(&goToAddressAction);
  menu.setToolTipsVisible(true);

  menu.exec(m_ui->registerView->viewport()->mapToGlobal(pos));
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
} // namespace Ripes
