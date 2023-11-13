#pragma once

#include "instruction.h"
#include "isainfo.h"
#include "processorhandler.h"
#include "pseudoinstruction.h"
#include "ripestab.h"

#include <QHeaderView>
#include <QTableView>
#include <QWidget>

namespace Ripes {

namespace Ui {
class SliderulesTab;
}

class SliderulesModel : public QAbstractTableModel {
  Q_OBJECT
public:
  SliderulesModel(
      const ISAInfoBase *isa,
      const std::shared_ptr<const InstrVec> instructions,
      const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
      QObject *parent = nullptr);
  ~SliderulesModel();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override = 0;

protected:
  const ISAInfoBase *m_isa;
  const std::shared_ptr<const InstrVec> m_instructions;
  const std::shared_ptr<const PseudoInstrVec> m_pseudoInstructions;
};

class EncodingModel final : public SliderulesModel {
  Q_OBJECT
public:
  EncodingModel(const ISAInfoBase *isa,
                const std::shared_ptr<const InstrVec> instructions,
                const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
                QObject *parent = nullptr);

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

private:
  QVariant instrData(size_t col, const InstructionBase *instr, int role) const;
};

class DecodingModel final : public SliderulesModel {
  Q_OBJECT
public:
  DecodingModel(const ISAInfoBase *isa,
                const std::shared_ptr<const InstrVec> instructions,
                const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
                QObject *parent = nullptr);

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
};

class SliderulesTab : public RipesTab {
  Q_OBJECT

public:
  static constexpr const int ROW_SIZE = 30;
  static constexpr const QSize MIN_CELL_SIZE = QSize(10, ROW_SIZE);
  static constexpr const QSize TYPE_CELL_SIZE = QSize(25, ROW_SIZE);
  static constexpr const QSize DESC_CELL_SIZE = QSize(100, ROW_SIZE);

  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

  void setData(const ISAInfoBase *isa, InstrVec instructions,
               PseudoInstrVec pseudoInstructions);
  void updateTables();

public slots:
  void onProcessorChanged();

private:
  void updateTable(QTableView *table, SliderulesModel *model);

  Ui::SliderulesTab *ui;
  const ISAInfoBase *m_isa;
  std::shared_ptr<const InstrVec> m_instructions;
  std::shared_ptr<const PseudoInstrVec> m_pseudoInstructions;
  std::unique_ptr<DecodingModel> m_decodingModel;
  std::unique_ptr<EncodingModel> m_encodingModel;
};

} // namespace Ripes
