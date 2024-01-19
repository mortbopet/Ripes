#include "sliderulesmodels.h"

#include "processorhandler.h"

namespace Ripes {

ISAEncodingTableModel::ISAEncodingTableModel(QObject *parent)
    : QAbstractTableModel(parent) {
  const auto isaInfo = ProcessorHandler::currentISA();
  setFamily(isaInfo->isaFamily());
}

void ISAEncodingTableModel::setFamily(ISAFamily family) {
  if (!m_isaInfo || m_isaInfo->isaFamily() != family) {
    m_isaInfo = ISAInfoRegistry::getISA(*ISAFamilySets.at(family).begin(),
                                        QStringList());
    emit familyChanged(family);
  }
}

int ISAEncodingTableModel::rowCount(const QModelIndex &) const { return 1; }

int ISAEncodingTableModel::columnCount(const QModelIndex &) const { return 1; }

QVariant ISAEncodingTableModel::data(const QModelIndex &index, int role) const {
  if (role == Qt::DisplayRole) {
    return QVariant::fromValue(m_isaInfo->isaFamily());
  }
  return QVariant();
}

} // namespace Ripes
