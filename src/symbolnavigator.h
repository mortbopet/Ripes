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
    SymbolNavigator(const Assembler::AddrOffsetMap& symbolmap, QWidget* parent = nullptr);
    ~SymbolNavigator();

    long getSelectedSymbolAddress() const;

private:
    void addSymbol(const long address, const QString& label);

    Ui::SymbolNavigator* m_ui;
};
}  // namespace Ripes
