#pragma once

#include "isainfo.h"

#include <QTableView>

namespace Ripes {

class ISAEncodingTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  ISAEncodingTableModel(QObject *parent = nullptr);

  // TODO(raccog): This could be a variable that is controlable in the UI.
  constexpr static size_t BIT_COLUMNS = 32;

  enum Column {
    EXTENSION = 0,
    TYPE,
    DESCRIPTION,
    OPCODE,
    // TODO(raccog): The field columns should be variable based on the maximum
    // number of fields
    FIELD0,
    FIELD1,
    FIELD2,
    BIT_START,
    BIT_END = BIT_START + BIT_COLUMNS
  };

  virtual int rowCount(const QModelIndex &) const override;
  virtual int columnCount(const QModelIndex &) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override;
  //  virtual QVariant headerData(int section, Qt::Orientation orientation,
  //                              int role = Qt::DisplayRole) const override;

public slots:
  void setISAInfo(std::shared_ptr<const ISAInfoBase> isaInfo);
  void setISAFamily(ISAFamily isaFamily);

signals:
  void isaInfoInitialized(const ISAInfoBase &isaInfo);
  void isaInfoChanged(const ISAInfoBase &isaInfo,
                      const ISAInfoBase &prevISAInfo);

protected:
  std::shared_ptr<const ISAInfoBase> m_isaInfo = nullptr;
  std::shared_ptr<const ISAInfoBase> m_prevIsaInfo = nullptr;
};

} // namespace Ripes
