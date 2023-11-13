#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"

namespace Ripes {

static constexpr const size_t EXTRA_COLS = 10;

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab),
      m_isa(ProcessorHandler::currentISA()),
      m_instructions(std::make_shared<const InstrVec>(
          ProcessorHandler::getAssembler()->getInstructionSet())),
      m_pseudoInstructions(std::make_shared<const PseudoInstrVec>(
          ProcessorHandler::getAssembler()->getPseudoInstructionSet())),
      m_decodingModel(std::make_unique<DecodingModel>(
          ProcessorHandler::currentISA(), m_instructions,
          m_pseudoInstructions)),
      m_encodingModel(std::make_unique<EncodingModel>(
          ProcessorHandler::currentISA(), m_instructions,
          m_pseudoInstructions)) {
  ui->setupUi(this);
  // TODO(raccog): Enable filtering of instructions
  ui->instrFilterInput->setReadOnly(true);

  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this,
          &SliderulesTab::onProcessorChanged);

  updateTables();
}

void SliderulesTab::onProcessorChanged() {
  const std::shared_ptr<Assembler::AssemblerBase> assembler =
      ProcessorHandler::getAssembler();
  const ISAInfoBase *isa = ProcessorHandler::currentISA();
  InstrVec instructions = assembler->getInstructionSet();
  PseudoInstrVec pseudoInstructions = assembler->getPseudoInstructionSet();
  setData(isa, instructions, pseudoInstructions);
}

void SliderulesTab::setData(const ISAInfoBase *isa, InstrVec instructions,
                            PseudoInstrVec pseudoInstructions) {
  m_isa = isa;
  m_instructions = std::make_shared<const InstrVec>(instructions);
  m_pseudoInstructions =
      std::make_shared<const PseudoInstrVec>(pseudoInstructions);

  m_decodingModel = std::make_unique<DecodingModel>(m_isa, m_instructions,
                                                    m_pseudoInstructions);
  m_encodingModel = std::make_unique<EncodingModel>(m_isa, m_instructions,
                                                    m_pseudoInstructions);

  updateTables();
}

void SliderulesTab::updateTable(QTableView *table, ISAModel *model) {
  table->setModel(model);
  table->horizontalHeader()->setMinimumSectionSize(MIN_CELL_SIZE.width());
  table->resizeColumnsToContents();
  table->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeMode::Stretch);
  table->verticalHeader()->hide();
  table->show();
}

void SliderulesTab::updateTables() {
  updateTable(ui->decodingTable, m_decodingModel.get());
  updateTable(ui->encodingTable, m_encodingModel.get());
}

SliderulesTab::~SliderulesTab() { delete ui; }

ISAModel::ISAModel(
    const ISAInfoBase *isa, const std::shared_ptr<const InstrVec> instructions,
    const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
    QObject *parent)
    : QAbstractTableModel(parent), m_isa(isa), m_instructions(instructions),
      m_pseudoInstructions(pseudoInstructions) {}

ISAModel::~ISAModel() {}

int ISAModel::rowCount(const QModelIndex &) const {
  return m_instructions->size() + m_pseudoInstructions->size();
}

int ISAModel::columnCount(const QModelIndex &) const {
  return m_isa->instrBits() + EXTRA_COLS;
}

EncodingModel::EncodingModel(
    const ISAInfoBase *isa, const std::shared_ptr<const InstrVec> instructions,
    const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
    QObject *parent)
    : ISAModel(isa, instructions, pseudoInstructions, parent) {}

QVariant EncodingModel::instrData(size_t col, const InstructionBase *instr,
                                  int role) const {
  switch (role) {
  case Qt::DisplayRole: {
    if (col == 0) {
      return instr->extensionOrigin();
    } else if (col == 1) {
      return "TYPE";
    } else if (col == 2) {
      return "DESCRIPTION";
    } else if (col == 3) {
      // Pseudo expanded ops
      return "";
    } else if (col == 4 || col == 5) {
      return "EXPLANATION";
    } else if (col == 6) {
      return instr->name();
    } else if (col >= 7 && col < EXTRA_COLS) {
      return "FIELDS";
    } else if (col >= EXTRA_COLS) {
      return QString::number(0);
    } else {
      return QVariant();
    }
  }
  case Qt::TextAlignmentRole:
    return Qt::AlignCenter;
  default:
    return QVariant();
  }
}

QVariant EncodingModel::data(const QModelIndex &index, int role) const {
  size_t row = static_cast<size_t>(index.row());
  size_t col = static_cast<size_t>(index.column());
  assert(row < m_instructions->size() + m_pseudoInstructions->size() &&
         "Cannot index past sliderule encoding model");
  if (row < m_instructions->size()) {
    auto instr = m_instructions->at(row);
    return instrData(col, instr.get(), role);
  } else {
    if (role == Qt::DisplayRole) {
      if (col == 0) {
        return "PSEUDO";
      } else if (col == 6) {
        return m_pseudoInstructions->at(index.row() - m_instructions->size())
            ->name();
      }
    }
  }
  return QVariant();
}

DecodingModel::DecodingModel(
    const ISAInfoBase *isa, const std::shared_ptr<const InstrVec> instructions,
    const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
    QObject *parent)
    : ISAModel(isa, instructions, pseudoInstructions, parent) {}

QVariant DecodingModel::data(const QModelIndex &index, int role) const {
  if (role == Qt::DisplayRole) {
  }
  return QVariant();
}

} // namespace Ripes
