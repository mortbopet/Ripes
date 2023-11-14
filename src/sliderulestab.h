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

namespace Cells {
struct CellStructure;
}

class ISAModel : public QAbstractTableModel {
  Q_OBJECT
public:
  ISAModel(const ISAInfoBase *isa,
           const std::shared_ptr<const InstrVec> instructions,
           const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
           QObject *parent = nullptr);
  ~ISAModel();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override = 0;

protected:
  void update();

  const ISAInfoBase *m_isa;
  const std::shared_ptr<const InstrVec> m_instructions;
  const std::shared_ptr<const PseudoInstrVec> m_pseudoInstructions;
  std::vector<std::unique_ptr<Cells::CellStructure>> m_encodingColumnStructure;
};

class EncodingModel final : public ISAModel {
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

class DecodingModel final : public ISAModel {
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
  static constexpr const QSize MIN_CELL_SIZE = QSize(10, 30);

  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

  void setData(const ISAInfoBase *isa, InstrVec instructions,
               PseudoInstrVec pseudoInstructions);
  void updateTables();

public slots:
  void onProcessorChanged();

private:
  void updateTable(QTableView *table, ISAModel *model);

  Ui::SliderulesTab *ui;
  const ISAInfoBase *m_isa;
  std::shared_ptr<const InstrVec> m_instructions;
  std::shared_ptr<const PseudoInstrVec> m_pseudoInstructions;
  std::unique_ptr<DecodingModel> m_decodingModel;
  std::unique_ptr<EncodingModel> m_encodingModel;
};

namespace Cells {

static constexpr size_t ROW_SIZE = 30;
static constexpr QSize BIT_SIZE = QSize(20, ROW_SIZE);
static constexpr QSize SMALL_SIZE = QSize(35, ROW_SIZE);
static constexpr QSize LARGE_SIZE = QSize(80, ROW_SIZE);

struct CellStructure {
  constexpr CellStructure(size_t columnIndex, size_t columnCount)
      : m_columnIndex(columnIndex), m_columnCount(columnCount) {}
  virtual QVariant getVariant(const InstructionBase *, int role) const {
    switch (role) {
    case Qt::DisplayRole:
      return m_display.has_value() ? m_display.value() : QVariant();
    case Qt::SizeHintRole:
      return m_sizeHint.has_value() ? m_sizeHint.value() : QVariant();
    case Qt::BackgroundRole:
      return m_background.has_value() ? m_background.value() : QVariant();
    case Qt::TextAlignmentRole:
      return m_alignment.has_value() ? m_alignment.value() : QVariant();
    default:
      return QVariant();
    }
  }

protected:
  unsigned bitIdx() const { return m_columnCount - m_columnIndex - 1; }
  std::optional<QString> m_display;
  std::optional<QBrush> m_background;
  std::optional<QSize> m_sizeHint;
  std::optional<Qt::AlignmentFlag> m_alignment = Qt::AlignCenter;
  size_t m_columnIndex;
  size_t m_columnCount;
};

} // namespace Cells

} // namespace Ripes
