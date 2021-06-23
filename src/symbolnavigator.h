#pragma once

#include <QDialog>

#include "assembler/objdump.h"

namespace Ripes {

namespace Ui {
class SymbolNavigator;
}

class SymbolNavigator : public QDialog {
    Q_OBJECT

public:
    SymbolNavigator(const ReverseSymbolMap& symbolmap, QWidget* parent = nullptr);
    ~SymbolNavigator();

    AInt getSelectedSymbolAddress() const;

private:
    void addSymbol(const AInt address, const QString& label);

    Ui::SymbolNavigator* m_ui;
};
}  // namespace Ripes
