#include "sliderulestab.h"
#include "processorhandler.h"
#include "ui_sliderulestab.h"

#include "isa/rv32isainfo.h"
#include "rv_i_ext.h"

namespace Ripes {

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab) {
  ui->setupUi(this);

  //  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged,
  //  this, &SliderulesTab::onProcessorChanged);
}

SliderulesTab::~SliderulesTab() { delete ui; }

EncodingModel::EncodingModel(
    const std::shared_ptr<const ISAInfoBase> isa,
    const std::shared_ptr<const InstrVec> instructions,
    const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
    QObject *parent)
    : QAbstractTableModel(parent), m_isa(isa), m_instructions(instructions),
      m_pseudoInstructions(pseudoInstructions) {}

int EncodingModel::rowCount(const QModelIndex &) const {
  return m_instructions->size();
}
int EncodingModel::columnCount(const QModelIndex &) const { return BIT_END; }

QVariant EncodingModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
  if (orientation == Qt::Horizontal) {
    switch (role) {
    case Qt::DisplayRole: {
      switch (section) {
      case EXTENSION:
        return QString("Extension");
      case TYPE:
        return QString("Type");
      case DESCRIPTION:
        return QString("Description");
      case EXPLANATION:
        return QString("Explanation");
      case OPCODE:
        return QString("Assembly");
      }
      if (section >= BIT_START && section < BIT_END) {
        return QString::number(BIT_END - section - 1);
      }
    }
    }
  }

  return QVariant();
}

QVariant fieldDisplay(const std::vector<std::shared_ptr<FieldBase>> &fields,
                      size_t idx) {
  if (fields.size() >= idx + 1)
    return QString(fields.at(idx)->fieldType());
  return QVariant();
}

QVariant EncodingModel::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 ||
      static_cast<size_t>(index.row()) >= m_instructions->size()) {
    return QVariant();
  }

  auto &instr = *m_instructions->at(index.row()).get();
  auto fields = instr.getFields();

  switch (role) {
  case Qt::DisplayRole: {
    auto col = index.column();
    switch (col) {
    case EXTENSION:
      return QString(instr.extensionOrigin());
    case TYPE:
      return "TODO: Type";
    case DESCRIPTION:
      return QString(instr.description());
    case EXPLANATION:
      return QString("TODO: Explanation");
    case OPCODE:
      return QString(instr.name());
    case FIELD0:
      return fieldDisplay(fields, 0);
    case FIELD1:
      return fieldDisplay(fields, 1);
    case FIELD2:
      return fieldDisplay(fields, 2);
    }

    if (col >= BIT_START && col < BIT_END) {
      unsigned bit = static_cast<unsigned>(BIT_END - col - 1);
      for (const auto &field : instr.getFields()) {
        for (const auto &range : field->ranges) {
          if (range.stop == bit) {
            return field->fieldType();
          }
        }
      }
      for (const auto &opPart : instr.getOpParts()) {
        if (opPart.range.isWithinRange(bit)) {
          return QString(opPart.bitIsSet(bit) ? "1" : "0");
        }
      }
    }
    break;
  }
  case Qt::TextAlignmentRole:
    return Qt::AlignCenter;
  }

  return QVariant();
}

EncodingView::EncodingView(QWidget *parent) : QTableView(parent) {
  auto isa = ProcessorHandler::fullISA();
  updateModel(isa);
  updateView();

  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this,
          &EncodingView::isaChanged);
}

void EncodingView::isaChanged() {
  updateModel(ProcessorHandler::fullISA());
  updateView();
}

void EncodingView::updateModel(std::shared_ptr<const ISAInfoBase> isa) {
  m_model = std::make_unique<EncodingModel>(
      isa, std::make_shared<const InstrVec>(isa->instructions()),
      std::make_shared<const PseudoInstrVec>(isa->pseudoInstructions()));
  setModel(m_model.get());
}

void EncodingView::updateView() {
  verticalHeader()->setVisible(false);
  horizontalHeader()->setMinimumSectionSize(30);
  size_t row = 0;
  clearSpans();
  for (const auto &instr : *m_model->m_instructions) {
    for (const auto &field : instr->getFields()) {
      for (const auto &range : field->ranges) {
        if (range.width() > 1) {
          setSpan(row, EncodingModel::BIT_END - range.stop - 1, 1,
                  range.width());
        }
      }
    }
    ++row;
  }
  resizeColumnsToContents();
}

DecodingModel::DecodingModel(const std::shared_ptr<const ISAInfoBase> isa,
                             const std::shared_ptr<const InstrVec> instructions,
                             QObject *parent)
    : QAbstractTableModel(parent), m_isa(isa), m_instructions(instructions) {}

int DecodingModel::rowCount(const QModelIndex &) const { return 0; }
int DecodingModel::columnCount(const QModelIndex &) const { return 0; }
QVariant DecodingModel::data(const QModelIndex &index, int role) const {
  return QVariant();
}

DecodingView::DecodingView(QWidget *parent) : QTableView(parent) {
  auto isa = ISAInfoRegistry::getISA<ISA::RV32I>(QStringList("M"));
  auto instructions = std::make_shared<InstrVec>();
  instructions->emplace_back(std::make_shared<RVISA::ExtI::TypeI::Addi>());
  m_model = std::make_unique<DecodingModel>(isa, instructions);
  setModel(m_model.get());
}

} // namespace Ripes
