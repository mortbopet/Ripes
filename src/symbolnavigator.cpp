#include "symbolnavigator.h"
#include "ui_symbolnavigator.h"

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QLineEdit>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QPushButton>

#include "processorhandler.h"
#include "radix.h"

namespace Ripes {

namespace {

class SymbolNavigatorModel : public QAbstractTableModel {
public:
  struct SymbolEntry {
    AInt address;
    QString label;
  };

  explicit SymbolNavigatorModel(QObject *parent = nullptr)
      : QAbstractTableModel(parent) {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const override {
    return parent.isValid() ? 0 : m_symbols.size();
  }

  int columnCount(const QModelIndex &parent = QModelIndex()) const override {
    return parent.isValid() ? 0 : 2;
  }

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
      return QVariant();
    }

    switch (section) {
    case 0:
      return "Address";
    case 1:
      return "Symbol";
    default:
      return QVariant();
    }
  }

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override {
    if (!index.isValid() || index.row() < 0 ||
        index.row() >= static_cast<int>(m_symbols.size())) {
      return QVariant();
    }

    const auto &entry = m_symbols.at(index.row());
    if (role == Qt::UserRole) {
      return QVariant::fromValue(entry.address);
    }
    if (role != Qt::DisplayRole) {
      return QVariant();
    }

    if (index.column() == 0) {
      return encodeRadixValue(entry.address, Radix::Hex,
                              ProcessorHandler::currentISA()->bytes());
    }
    if (index.column() == 1) {
      return entry.label;
    }
    return QVariant();
  }

  void addSymbol(AInt address, const QString &label) {
    const int row = m_symbols.size();
    beginInsertRows(QModelIndex(), row, row);
    m_symbols.push_back({address, label});
    endInsertRows();
  }

private:
  QVector<SymbolEntry> m_symbols;
};

class SymbolFilterProxyModel : public QSortFilterProxyModel {
public:
  explicit SymbolFilterProxyModel(QObject *parent = nullptr)
      : QSortFilterProxyModel(parent) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
  }

protected:
  bool filterAcceptsRow(int sourceRow,
                        const QModelIndex &sourceParent) const override {
    const auto *model = sourceModel();
    if (!model) {
      return true;
    }

    const auto filterText = filterRegularExpression().pattern();
    if (filterText.isEmpty()) {
      return true;
    }

    for (int column = 0; column < model->columnCount(sourceParent); ++column) {
      const auto data = model->index(sourceRow, column, sourceParent)
                            .data(filterRole())
                            .toString();
      if (data.contains(filterRegularExpression())) {
        return true;
      }
    }
    return false;
  }
};

} // namespace

SymbolNavigator::SymbolNavigator(const ReverseSymbolMap &symbolmap,
                                 QWidget *parent)
    : QDialog(parent), m_ui(new Ui::SymbolNavigator) {
  m_ui->setupUi(this);

  setWindowTitle("Symbol navigator");

  m_model = new SymbolNavigatorModel(this);
  for (const auto &iter : symbolmap) {
    static_cast<SymbolNavigatorModel *>(m_model)->addSymbol(iter.first,
                                                            iter.second);
  }

  m_proxyModel = new SymbolFilterProxyModel(this);
  m_proxyModel->setSourceModel(m_model);
  m_proxyModel->setFilterKeyColumn(-1);
  m_ui->symbolTable->setModel(m_proxyModel);
  m_ui->symbolTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_ui->symbolTable->setSelectionMode(QAbstractItemView::SingleSelection);
  m_ui->symbolTable->verticalHeader()->hide();
  m_ui->symbolTable->horizontalHeader()->setStretchLastSection(true);
  m_ui->symbolTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  m_ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Go to symbol");

  connect(m_ui->filterEdit, &QLineEdit::textChanged, this,
          [this](const QString &text) {
            m_proxyModel->setFilterRegularExpression(
                QRegularExpression(QRegularExpression::escape(text),
                                   QRegularExpression::CaseInsensitiveOption));
            if (m_proxyModel->rowCount() > 0) {
              m_ui->symbolTable->selectRow(0);
            }
          });

  if (m_proxyModel->rowCount() > 0) {
    m_ui->symbolTable->selectRow(0);
  }
}

AInt SymbolNavigator::getSelectedSymbolAddress() const {
  const auto index = m_ui->symbolTable->currentIndex();
  if (!index.isValid()) {
    return 0;
  }
  const auto sourceIndex = m_proxyModel->mapToSource(index);
  if (!sourceIndex.isValid()) {
    return 0;
  }
  return sourceIndex.data(Qt::UserRole).toUInt();
}

SymbolNavigator::~SymbolNavigator() { delete m_ui; }
} // namespace Ripes
