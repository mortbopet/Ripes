#include "symbolnavigator.h"
#include "ui_symbolnavigator.h"

#include "processorhandler.h"
#include "radix.h"

#include <QPushButton>

namespace Ripes {

SymbolNavigator::SymbolNavigator(const ReverseSymbolMap& symbolmap, QWidget* parent)
    : QDialog(parent), m_ui(new Ui::SymbolNavigator) {
    m_ui->setupUi(this);

    setWindowTitle("Symbol navigator");

    m_ui->symbolTable->setColumnCount(2);
    m_ui->symbolTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->symbolTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui->symbolTable->verticalHeader()->hide();
    m_ui->symbolTable->setHorizontalHeaderLabels({"Address", "Symbol"});
    m_ui->symbolTable->horizontalHeader()->setStretchLastSection(true);
    m_ui->symbolTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Go to symbol");

    for (const auto& iter : symbolmap) {
        addSymbol(iter.first, iter.second);
    }
    m_ui->symbolTable->selectRow(0);
}

AInt SymbolNavigator::getSelectedSymbolAddress() const {
    const auto selected = m_ui->symbolTable->selectedItems();
    if (selected.size() > 0) {
        return selected[0]->data(Qt::UserRole).toUInt();
    }
    return 0;
}

void SymbolNavigator::addSymbol(const AInt address, const QString& label) {
    m_ui->symbolTable->setRowCount(m_ui->symbolTable->rowCount() + 1);
    QTableWidgetItem* addrItem = new QTableWidgetItem();
    QTableWidgetItem* labelItem = new QTableWidgetItem();

    addrItem->setData(Qt::DisplayRole, encodeRadixValue(address, Radix::Hex, ProcessorHandler::currentISA()->bytes()));
    labelItem->setData(Qt::DisplayRole, label);

    addrItem->setFlags(addrItem->flags() ^ Qt::ItemIsEditable);
    labelItem->setFlags(labelItem->flags() ^ Qt::ItemIsEditable);

    addrItem->setData(Qt::UserRole, QVariant::fromValue(address));
    labelItem->setData(Qt::UserRole, QVariant::fromValue(address));

    const int row = m_ui->symbolTable->rowCount() - 1;
    m_ui->symbolTable->setItem(row, 0, addrItem);
    m_ui->symbolTable->setItem(row, 1, labelItem);
}

SymbolNavigator::~SymbolNavigator() {
    delete m_ui;
}
}  // namespace Ripes
