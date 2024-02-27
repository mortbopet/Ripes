#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"

namespace Ripes {

InstructionItem::InstructionItem(const InstructionBase &instruction,
                                 InstructionItem *parent)
    : m_instruction(instruction), m_parent(parent) {
  if (!parent)
    m_children.push_back(std::make_unique<InstructionItem>(instruction, this));
}

InstructionItem *InstructionItem::child() {
  return m_children.size() > 0 ? m_children.at(0).get() : nullptr;
}

const InstructionBase &InstructionItem::data() const { return m_instruction; }

InstructionItem *InstructionItem::parent() { return m_parent; }

bool InstructionItem::hasParent() const { return m_parent; }

ISAInstructionsModel::ISAInstructionsModel(QObject *parent) {
  for (const auto &instr : ProcessorHandler::currentISA()->instructions()) {
    m_instrItems.push_back(std::make_unique<InstructionItem>(*instr.get()));
  }
}

int ISAInstructionsModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return parent.parent().isValid() ? 0 : 1;
  }
  return m_instrItems.size();
}

int ISAInstructionsModel::columnCount(const QModelIndex &) const { return 1; }

QVariant ISAInstructionsModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || role != Qt::DisplayRole) {
    return {};
  }

  const auto *item =
      static_cast<const InstructionItem *>(index.internalPointer());
  return !item->hasParent() ? item->data().name() : item->data().description();
}

Qt::ItemFlags ISAInstructionsModel::flags(const QModelIndex &index) const {
  return index.isValid() ? QAbstractItemModel::flags(index)
                         : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant ISAInstructionsModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  return {};
}

QModelIndex ISAInstructionsModel::index(int row, int column,
                                        const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) {
    return {};
  }

  if (parent.isValid()) {
    if (auto *child =
            static_cast<InstructionItem *>(parent.internalPointer())->child()) {
      return createIndex(row, column, child);
    }
  } else {
    if (static_cast<size_t>(row) < m_instrItems.size()) {
      return createIndex(row, column, m_instrItems.at(row).get());
    }
  }

  return {};
}

QModelIndex ISAInstructionsModel::parent(const QModelIndex &index) const {
  if (!index.isValid()) {
    return {};
  }

  auto *item = static_cast<InstructionItem *>(index.internalPointer());
  auto *parent = item->parent();

  return parent ? createIndex(0, 0, parent) : QModelIndex{};
}

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab) {
  ui->setupUi(this);

  isaFamilyBox = ui->isaFamilyBox;
  isaBox = ui->isaBox;
  mainExtBox = ui->mainExtBox;
  baseExtCheckBox = ui->baseExtCheckBox;

  m_isaModel = std::make_unique<ISAInstructionsModel>();
  ui->encodingTable->setModel(m_isaModel.get());

  for (const auto &familyName : ISAFamilyNames) {
    isaFamilyBox->addItem(familyName.second,
                          QVariant(static_cast<int>(familyName.first)));
  }

  const auto &isaInfo = *ISAInfoRegistry::getISA(ISA::RV32I, QStringList());
  isaFamilyBox->setCurrentIndex(
      isaFamilyBox->findData(QVariant(static_cast<int>(isaInfo.isaFamily()))));

  isaBox->clear();
  for (const auto &isa : ISAFamilySets.at(isaInfo.isaFamily())) {
    isaBox->addItem(ISANames.at(isa), QVariant(static_cast<int>(isa)));
  }

  isaBox->setCurrentIndex(
      isaBox->findData(QVariant(static_cast<int>(isaInfo.isaID()))));

  mainExtBox->clear();
  for (const auto &ext :
       QStringList(isaInfo.baseExtension()) + isaInfo.supportedExtensions()) {
    mainExtBox->addItem(ext, ext);
  }

  baseExtCheckBox->setText(isaInfo.baseExtension());
  baseExtCheckBox->setChecked(true);
  baseExtCheckBox->setEnabled(false);
}

SliderulesTab::~SliderulesTab() { delete ui; }

} // namespace Ripes
