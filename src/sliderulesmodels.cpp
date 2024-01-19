#include "sliderulesmodels.h"

#include "processorhandler.h"

namespace Ripes {

ISAEncodingTableModel::ISAEncodingTableModel(QObject *parent)
    : QAbstractTableModel(parent) {}

void ISAEncodingTableModel::setFamily(ISAFamily family) {
  if (!m_isaInfo || m_isaInfo->isaFamily() != family) {
    m_isaInfo = ISAInfoRegistry::getISA(*ISAFamilySets.at(family).begin(),
                                        QStringList());
    emit familyChanged(family);
  }
}

void ISAEncodingTableModel::setISA(ISA isa) {
  if (!m_isaInfo || m_isaInfo->isaID() != isa) {
    auto extensions = QStringList();
    // NOTE: This assumes that all ISAs within a single family have the same
    // supported extensions. Change this logic if that statement is not true.
    if (m_isaInfo && m_isaInfo->isaFamily() == ISAFamilies.at(isa)) {
      extensions = m_isaInfo->enabledExtensions();
    }
    m_isaInfo = ISAInfoRegistry::getISA(isa, extensions);
    emit isaChanged(isa);
  }
}

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

} // namespace Ripes
