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
  /// Updates the Model and emits `familyChanged()` signal if the model's ISA
  /// family has changed.
  ///
  /// This should only be `connect()`ed to UI signals whose function names end
  /// in `Activated`. Can also be `connect()`ed to
  /// `ProcessorHandler::processorChanged()`.
  void setFamily(ISAFamily family);
  void setISA(ISA isa);

signals:
  /// Called when the ISA family of the model changes. Use this to update UI.
  void familyChanged(ISAFamily family);
  /// Called when the ISA of the model changes. Use this to update UI.
  void isaChanged(ISA isa);

protected:
  std::shared_ptr<const ISAInfoBase> m_isaInfo = nullptr;
};

} // namespace Ripes
