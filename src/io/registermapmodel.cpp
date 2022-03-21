#include "registermapmodel.h"

#include <QHeaderView>

#include "fonts.h"
#include "iobase.h"
#include "processorhandler.h"

namespace Ripes {

RegisterMapModel::RegisterMapModel(QPointer<IOBase> peripheral, QObject *parent)
    : QAbstractTableModel(parent), m_peripheral(peripheral) {
  connect(m_peripheral, &IOBase::regMapChanged, this,
          &RegisterMapModel::regMapChanged);
}

void RegisterMapModel::regMapChanged() {
  // Let's assume that this is not a hot function, and just update the whole
  // view.
  emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
  emit layoutChanged();
}

int RegisterMapModel::columnCount(const QModelIndex &) const {
  return NColumns;
}

int RegisterMapModel::rowCount(const QModelIndex &) const {
  if (!m_peripheral.isNull()) {
    return m_peripheral->registers().size();
  } else {
    return 0;
  }
}

QVariant RegisterMapModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case Column::Name:
      return "Name";
    case Column::Address:
      return "Address";
    case Column::RW:
      return "R/W?";
    case Column::BitWidth:
      return "Size";
    default:
      return QVariant();
    }
  }
  return QVariant();
}

QVariant RegisterMapModel::data(const QModelIndex &index, int role) const {
  if (m_peripheral.isNull()) {
    return QVariant();
  }

  if (!index.isValid()) {
    return QVariant();
  }

  const unsigned idx = index.row();
  switch (index.column()) {
  case Column::Name: {
    if (role == Qt::DisplayRole) {
      return m_peripheral->registers().at(idx).name;
    }
    break;
  }
  case Column::Address: {
    switch (role) {
    case Qt::DisplayRole:
      return "0x" +
             QString::number(m_peripheral->registers().at(idx).address, 16);
    case Qt::FontRole:
      return QFont(Fonts::monospace, 11);
    case Qt::TextAlignmentRole:
      return Qt::AlignCenter;
    }
    break;
  }
  case Column::RW: {
    switch (role) {
    case Qt::DisplayRole: {
      switch (m_peripheral->registers().at(idx).rw) {
      case RegDesc::RW::R:
        return "R";
      case RegDesc::RW::W:
        return "W";
      case RegDesc::RW::RW:
        return "R/W";
      default:
        Q_UNREACHABLE();
      }
    }
    case Qt::TextAlignmentRole:
      return Qt::AlignCenter;
    }
    break;
  }

  case Column::BitWidth: {
    switch (role) {
    case Qt::DisplayRole:
      return QString::number(m_peripheral->registers().at(idx).bitWidth);
    case Qt::TextAlignmentRole:
      return Qt::AlignCenter;
    }
    break;
  }
  default:
    return QVariant();
  }
  return QVariant();
}

Qt::ItemFlags RegisterMapModel::flags(const QModelIndex &) const {
  return Qt::ItemIsEnabled;
}
} // namespace Ripes
