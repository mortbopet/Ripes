#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"

namespace Ripes {

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab),
      m_isa(ProcessorHandler::currentISA()),
      m_instructions(std::make_shared<const InstrVec>(
          ProcessorHandler::getAssembler()->getInstructionSet())),
      m_pseudoInstructions(std::make_shared<const PseudoInstrVec>(
          ProcessorHandler::getAssembler()->getPseudoInstructionSet())),
      m_decodingModel(std::make_unique<SliderulesDecodingModel>(
          ProcessorHandler::currentISA(), m_instructions,
          m_pseudoInstructions)),
      m_encodingModel(std::make_unique<SliderulesEncodingModel>(
          ProcessorHandler::currentISA(), m_instructions,
          m_pseudoInstructions)) {
  ui->setupUi(this);

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

  m_decodingModel = std::make_unique<SliderulesDecodingModel>(
      m_isa, m_instructions, m_pseudoInstructions);
  m_encodingModel = std::make_unique<SliderulesEncodingModel>(
      m_isa, m_instructions, m_pseudoInstructions);

  updateTables();
}

void SliderulesTab::updateTables() {
  ui->decodingTable->setModel(m_decodingModel.get());
  ui->decodingTable->show();
  ui->encodingTable->setModel(m_encodingModel.get());
  ui->encodingTable->show();
}

SliderulesTab::~SliderulesTab() { delete ui; }

SliderulesModel::SliderulesModel(
    const ISAInfoBase *isa, const std::shared_ptr<const InstrVec> instructions,
    const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
    QObject *parent)
    : QAbstractTableModel(parent), m_isa(isa), m_instructions(instructions),
      m_pseudoInstructions(pseudoInstructions) {}

SliderulesModel::~SliderulesModel() {}

int SliderulesModel::rowCount(const QModelIndex &) const {
  return m_instructions->size() + m_pseudoInstructions->size();
}

int SliderulesModel::columnCount(const QModelIndex &) const {
  return m_isa->instrBits() + 10;
}

SliderulesEncodingModel::SliderulesEncodingModel(
    const ISAInfoBase *isa, const std::shared_ptr<const InstrVec> instructions,
    const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
    QObject *parent)
    : SliderulesModel(isa, instructions, pseudoInstructions, parent) {}

QVariant SliderulesEncodingModel::data(const QModelIndex &index,
                                       int role) const {
  if (role == Qt::DisplayRole) {
    if (index.column() == 0) {
      assert(static_cast<size_t>(index.row()) <
                 m_instructions->size() + m_pseudoInstructions->size() &&
             "Cannot index past sliderule encoding model");
      return (static_cast<size_t>(index.row()) < m_instructions->size())
                 ? m_instructions->at(index.row())->name()
                 : m_pseudoInstructions
                       ->at(index.row() - m_instructions->size())
                       ->name();
    }
    return QString::number(index.row()) + " " + QString::number(index.column());
  }
  return QVariant();
}

SliderulesDecodingModel::SliderulesDecodingModel(
    const ISAInfoBase *isa, const std::shared_ptr<const InstrVec> instructions,
    const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
    QObject *parent)
    : SliderulesModel(isa, instructions, pseudoInstructions, parent) {}

QVariant SliderulesDecodingModel::data(const QModelIndex &index,
                                       int role) const {
  if (role == Qt::DisplayRole) {
  }
  return QVariant();
}

} // namespace Ripes
