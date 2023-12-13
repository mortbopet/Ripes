#pragma once

#include "instruction.h"
#include "isainfo.h"
#include "pseudoinstruction.h"
#include "ripestab.h"

#include <QHeaderView>
#include <QTableView>
#include <QWidget>

namespace Ripes {

namespace Ui {
class SliderulesTab;
}

class EncodingView;

class EncodingModel : public QAbstractTableModel {
  Q_OBJECT
public:
  EncodingModel(const std::shared_ptr<const ISAInfoBase> isa,
                const std::shared_ptr<const InstrVec> instructions,
                const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
                QObject *parent = nullptr);

  enum Column {
    EXTENSION = 0,
    TYPE = 1,
    DESCRIPTION = 2,
    EXPLANATION = 3,
    OPCODE = 4,
    FIELD0 = 5,
    FIELD1 = 6,
    FIELD2 = 7,
    BIT_START = 8,
    BIT_END = BIT_START + 32
  };

  virtual int rowCount(const QModelIndex &) const override;
  virtual int columnCount(const QModelIndex &) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const override;

protected:
  friend class EncodingView;

  size_t m_rows;

  const std::shared_ptr<const ISAInfoBase> m_isa;
  const std::shared_ptr<const InstrVec> m_instructions;
  const std::shared_ptr<const PseudoInstrVec> m_pseudoInstructions;

  std::map<size_t, const InstructionBase *> m_rowInstrMap;
};

class EncodingView : public QTableView {
  Q_OBJECT
public:
  EncodingView(QWidget *parent = nullptr);

public slots:
  void isaChanged();

protected:
  void updateModel(std::shared_ptr<const ISAInfoBase> isa);
  void updateView();

  std::unique_ptr<EncodingModel> m_model;
};

struct DecodingModel : public QAbstractTableModel {
  DecodingModel(const std::shared_ptr<const ISAInfoBase> isa,
                const std::shared_ptr<const InstrVec> instructions,
                QObject *parent = nullptr);

  virtual int rowCount(const QModelIndex &) const override;
  virtual int columnCount(const QModelIndex &) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override;

protected:
  const std::shared_ptr<const ISAInfoBase> m_isa;
  const std::shared_ptr<const InstrVec> m_instructions;
};

class DecodingView : public QTableView {
  Q_OBJECT
public:
  DecodingView(QWidget *parent = nullptr);

protected:
  std::unique_ptr<DecodingModel> m_model;
};

class SliderulesTab : public RipesTab {
  Q_OBJECT
public:
  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

protected:
private:
  Ui::SliderulesTab *ui;
};

} // namespace Ripes
