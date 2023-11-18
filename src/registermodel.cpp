#include "registermodel.h"

#include <QHeaderView>

#include "STLExtras.h"
#include "fonts.h"
#include "processorhandler.h"

namespace Ripes {

using namespace vsrtl;

RegisterModel::RegisterModel(const std::string_view &rft, QObject *parent)
    : QAbstractTableModel(parent), m_rft(rft) {
  m_regBytes = ProcessorHandler::getProcessor()->implementsISA()->bytes();
}

std::vector<VInt> RegisterModel::gatherRegisterValues() {
  std::vector<VInt> vals;
  for (int i = 0; i < rowCount(); ++i)
    vals.push_back(ProcessorHandler::getRegisterValue(m_rft, i));
  return vals;
}

int RegisterModel::columnCount(const QModelIndex &) const { return NColumns; }

int RegisterModel::rowCount(const QModelIndex &) const {
  if (auto regInfo = ProcessorHandler::currentISA()->regInfo(m_rft);
      regInfo.has_value()) {
    return (*regInfo)->regCnt();
  } else {
    return 0;
  }
}

void RegisterModel::processorWasClocked() {
  // Reload model
  beginResetModel();
  endResetModel();
  const auto newRegValues = gatherRegisterValues();
  if (m_regValues.size() != 0) {
    for (auto newRegVal : llvm::enumerate(newRegValues)) {
      if (m_regValues[newRegVal.index()] != newRegVal.value()) {
        m_mostRecentlyModifiedReg = newRegVal.index();
        emit registerChanged(newRegVal.index());
        break;
      }
    }
  }
  m_regValues = newRegValues;
}

bool RegisterModel::setData(const QModelIndex &index, const QVariant &value,
                            int) {
  const int i = index.row();
  if (index.column() == Column::Value) {
    bool ok;
    VInt v = decodeRadixValue(value.toString(), m_radix, &ok);
    if (ok) {
      ProcessorHandler::setRegisterValue(m_rft, i, v);
      emit dataChanged(index, index);
      return true;
    }
  }
  return false;
}

QVariant RegisterModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case Column::Name:
      return "Name";
    case Column::Alias:
      return "Alias";
    case Column::Value:
      return "Value";
    default:
      return QVariant();
    }
  }
  return QVariant();
}

QVariant RegisterModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  const unsigned idx = index.row();
  if (role == Qt::ToolTipRole) {
    return tooltipData(idx);
  } else if (role == Qt::BackgroundRole &&
             index.row() == m_mostRecentlyModifiedReg) {
    return QBrush(QColor{0xFD, 0xB5, 0x15});
  }

  switch (index.column()) {
  case Column::Name: {
    if (role == Qt::DisplayRole) {
      return nameData(idx);
    }
    break;
  }
  case Column::Alias: {
    if (role == Qt::DisplayRole) {
      return aliasData(idx);
    }
    break;
  }
  case Column::Value: {
    switch (role) {
    case Qt::DisplayRole:
      return valueData(idx);
    case Qt::UserRole:
      return QVariant::fromValue(m_radix);
    case Qt::FontRole:
      return QFont(Fonts::monospace, 11);
    case Qt::ForegroundRole:
      return QBrush(Qt::blue);
    case Qt::EditRole:
      return QVariant::fromValue(registerData(idx));
    default:
      break;
    }
  }
  }
  return QVariant();
}

void RegisterModel::setRadix(Ripes::Radix r) {
  m_radix = r;
  processorWasClocked();
}

QVariant RegisterModel::nameData(unsigned idx) const {
  if (auto regInfo = ProcessorHandler::currentISA()->regInfo(m_rft);
      regInfo.has_value()) {
    return (*regInfo)->regName(idx);
  } else {
    return QVariant();
  }
}

QVariant RegisterModel::aliasData(unsigned idx) const {
  if (auto regInfo = ProcessorHandler::currentISA()->regInfo(m_rft);
      regInfo.has_value()) {
    return (*regInfo)->regAlias(idx);
  } else {
    return QVariant();
  }
}

QVariant RegisterModel::tooltipData(unsigned idx) const {
  if (auto regInfo = ProcessorHandler::currentISA()->regInfo(m_rft);
      regInfo.has_value()) {
    return (*regInfo)->regInfo(idx);
  } else {
    return QVariant();
  }
}

VInt RegisterModel::registerData(unsigned idx) const {
  return ProcessorHandler::getRegisterValue(m_rft, idx);
}

QVariant RegisterModel::valueData(unsigned idx) const {
  return encodeRadixValue(registerData(idx), m_radix, m_regBytes);
}

Qt::ItemFlags RegisterModel::flags(const QModelIndex &index) const {
  if (auto regInfo = ProcessorHandler::currentISA()->regInfo(m_rft);
      regInfo.has_value()) {
    const auto def = (*regInfo)->regIsReadOnly(index.row()) ? Qt::NoItemFlags
                                                            : Qt::ItemIsEnabled;
    if (index.column() == Column::Value)
      return Qt::ItemIsEditable | def;
    return def;
  } else {
    return Qt::NoItemFlags;
  }
}
} // namespace Ripes
