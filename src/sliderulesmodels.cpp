#include "sliderulesmodels.h"

#include "processorhandler.h"

namespace Ripes {

ISAEncodingTableModel::ISAEncodingTableModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int ISAEncodingTableModel::rowCount(const QModelIndex &) const { return 1; }

int ISAEncodingTableModel::columnCount(const QModelIndex &) const { return 1; }

QVariant ISAEncodingTableModel::data(const QModelIndex &index, int role) const {
  if (!m_isaInfo) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return ISAFamilyNames.at(m_isaInfo->isaFamily()) + " " +
           ISANames.at(m_isaInfo->isaID());
  }
  return QVariant();
}

void ISAEncodingTableModel::setISAInfo(
    std::shared_ptr<const ISAInfoBase> isaInfo) {
  bool initialize = m_isaInfo == nullptr;
  m_prevIsaInfo = initialize ? isaInfo : m_isaInfo;
  m_isaInfo = isaInfo;
  if (initialize) {
    emit isaInfoInitialized(*m_isaInfo.get());
  } else {
    emit isaInfoChanged(*m_isaInfo.get(), *m_prevIsaInfo.get());
  }
}

void ISAEncodingTableModel::setISAFamily(ISAFamily isaFamily) {
  if (m_isaInfo->isaFamily() != isaFamily) {
    setISAInfo(ISAInfoRegistry::getISA(*ISAFamilySets.at(isaFamily).begin(),
                                       QStringList()));
  }
}

} // namespace Ripes
