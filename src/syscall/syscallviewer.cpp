#include "syscallviewer.h"
#include "ui_syscallviewer.h"

#include "processorhandler.h"

#include <QTableWidgetItem>

namespace Ripes {

SyscallViewer::SyscallViewer(QWidget* parent) : QDialog(parent), m_ui(new Ui::SyscallViewer) {
    m_ui->setupUi(this);

    setWindowTitle("Supported system calls");

    m_ui->splitter->setStretchFactor(0, 0);
    m_ui->splitter->setStretchFactor(1, 1);

    auto font = m_ui->syscallTitle->font();
    font.setBold(true);
    font.setPointSize(static_cast<int>(font.pointSize() * 1.2));
    m_ui->syscallTitle->setFont(font);

    const auto& syscallManager = ProcessorHandler::get()->getSyscallManager();

    // Setup syscall list
    m_ui->syscallList->setColumnCount(2);
    m_ui->syscallList->setSortingEnabled(false);
    for (const auto& iter : syscallManager.getSyscalls()) {
        addSyscall(iter.first, iter.second.get());
    }
    m_ui->syscallList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->syscallList->verticalHeader()->hide();
    m_ui->syscallList->setHorizontalHeaderLabels(
        {"Func. (" +
             ProcessorHandler::get()->currentISA()->regAlias(ProcessorHandler::get()->currentISA()->syscallReg()) + ")",
         "Name"});
    m_ui->syscallList->horizontalHeader()->setStretchLastSection(true);
    connect(m_ui->syscallList, &QTableWidget::currentItemChanged, this, &SyscallViewer::handleItemChanged);

    // Setup syscall argument styling
    m_ui->syscallArguments->setColumnCount(2);
    m_ui->syscallArguments->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->syscallArguments->horizontalHeader()->setStretchLastSection(true);
    m_ui->syscallArguments->setHorizontalHeaderLabels({"Arg. #", "Description"});
    m_ui->syscallArguments->verticalHeader()->hide();

    // Setup syscall returns styling
    m_ui->syscallReturns->setColumnCount(2);
    m_ui->syscallReturns->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->syscallReturns->horizontalHeader()->setStretchLastSection(true);
    m_ui->syscallReturns->setHorizontalHeaderLabels({"Ret. #", "Description"});
    m_ui->syscallReturns->verticalHeader()->hide();

    // Select first row to set the initial state
    m_ui->syscallList->selectRow(0);
}

void SyscallViewer::handleItemChanged(QTableWidgetItem* current, QTableWidgetItem*) {
    if (current == nullptr)
        return;

    setCurrentSyscall(current->data(Qt::UserRole).value<const Syscall*>());
}

void SyscallViewer::setCurrentSyscall(const Syscall* syscall) {
    if (syscall == nullptr)
        return;

    m_ui->syscallArguments->clearContents();
    m_ui->syscallArguments->setRowCount(0);
    m_ui->syscallReturns->clearContents();
    m_ui->syscallReturns->setRowCount(0);

    m_ui->syscallTitle->setText(syscall->name());
    m_ui->syscallDescription->setText(syscall->description());
    for (const auto& iter : syscall->argumentDescriptions()) {
        addItemToTable(m_ui->syscallArguments, iter.first, iter.second);
    }
    for (const auto& iter : syscall->returnDescriptions()) {
        addItemToTable(m_ui->syscallReturns, iter.first, iter.second);
    }
}

void SyscallViewer::addItemToTable(QTableWidget* table, unsigned int idx, const QString& description) {
    table->setRowCount(table->rowCount() + 1);
    QTableWidgetItem* idxItem = new QTableWidgetItem();
    QTableWidgetItem* textItem = new QTableWidgetItem();

    idxItem->setData(Qt::EditRole, QString::number(idx));
    textItem->setData(Qt::DisplayRole, description);

    idxItem->setFlags(idxItem->flags() ^ Qt::ItemIsEditable);
    textItem->setFlags(textItem->flags() ^ Qt::ItemIsEditable);

    const int row = table->rowCount() - 1;
    table->setItem(row, 0, idxItem);
    table->setItem(row, 1, textItem);
}

void SyscallViewer::addSyscall(unsigned id, const Syscall* syscall) {
    m_ui->syscallList->setRowCount(m_ui->syscallList->rowCount() + 1);
    QTableWidgetItem* idItem = new QTableWidgetItem();
    QTableWidgetItem* textItem = new QTableWidgetItem();

    idItem->setData(Qt::EditRole, QString::number(id));
    textItem->setData(Qt::DisplayRole, syscall->name());
    idItem->setData(Qt::UserRole, QVariant::fromValue(syscall));
    textItem->setData(Qt::UserRole, QVariant::fromValue(syscall));

    idItem->setFlags(idItem->flags() ^ Qt::ItemIsEditable);
    textItem->setFlags(textItem->flags() ^ Qt::ItemIsEditable);

    const int row = m_ui->syscallList->rowCount() - 1;
    m_ui->syscallList->setItem(row, 0, idItem);
    m_ui->syscallList->setItem(row, 1, textItem);
}

SyscallViewer::~SyscallViewer() {
    delete m_ui;
}

}  // namespace Ripes

Q_DECLARE_METATYPE(const Ripes::Syscall*);
