#pragma once

#include <QDialog>

#include "assembler/objdump.h"

QT_FORWARD_DECLARE_CLASS(QSortFilterProxyModel)
QT_FORWARD_DECLARE_CLASS(QAbstractTableModel)

namespace Ripes {

namespace Ui {
class SymbolNavigator;
}

class SymbolNavigator : public QDialog {
  Q_OBJECT

public:
  SymbolNavigator(const ReverseSymbolMap &symbolmap, QWidget *parent = nullptr);
  ~SymbolNavigator();

  AInt getSelectedSymbolAddress() const;

private:
  Ui::SymbolNavigator *m_ui;
  QAbstractTableModel *m_model = nullptr;
  QSortFilterProxyModel *m_proxyModel = nullptr;
};
} // namespace Ripes
