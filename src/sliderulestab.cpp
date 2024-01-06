#include "sliderulestab.h"
#include "processorhandler.h"
#include "ui_sliderulestab.h"

#include "rv_i_ext.h"

namespace Ripes {

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    // TODO(raccog): Initialize default selected ISA using cached settings
    : RipesTab(toolbar, parent), m_selectedISA(ISA::RV32I),
      ui(new Ui::SliderulesTab) {
  ui->setupUi(this);

  // Create encoding table and transfer ownership to UI layout.
  m_encodingTable = new EncodingView(*ui->isaSelector);
  ui->encodingLayout->addWidget(m_encodingTable);

  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this,
          &SliderulesTab::processorChanged);
  connect(ui->isaSelector, &QComboBox::currentIndexChanged, this,
          &SliderulesTab::updateRegWidthSelector);
  connect(ui->regWidthSelector, &QComboBox::currentIndexChanged, this,
          &SliderulesTab::isaSelectorChanged);
  connect(ui->isaSelector, &QComboBox::currentIndexChanged, this,
          &SliderulesTab::isaSelectorChanged);
  connect(ui->mainExtensionSelector, &QComboBox::currentIndexChanged, this,
          &SliderulesTab::isaSelectorChanged);

  updateISASelector(true);
}

void SliderulesTab::isaSelectorChanged() {
  auto isa = m_encodingTable->model->isa;
  auto exts = QStringList();

  // Get name of main extension; exit early if it has not been filled in yet
  auto mainExt = ui->mainExtensionSelector->currentText();
  if (mainExt.isEmpty()) {
    return;
  }

  // Set all rows to visible
  for (int i = 0; i < m_encodingTable->model->rowCount(QModelIndex()); ++i) {
    m_encodingTable->setRowHidden(i, false);
  }

  // Check if base extension should be hidden and get selected extensions
  bool hideBaseExt = false;
  if (isa->supportsExtension(mainExt)) {
    exts = QStringList() << mainExt;
    hideBaseExt = true;
  }

  // Update tables with new ISA
  m_encodingTable->updateISA(ui->regWidthSelector->currentText(), exts);

  // Hide base extension instruction rows in encoding table if necessary
  //  if (hideBaseExt) {
  //    for (const auto &instr : *ui->encodingTable->model->instructions) {
  //      if (instr.second->extensionOrigin() == isa->baseExtension()) {
  //        ui->encodingTable->setRowHidden(instr.first, true);

  //        // Hide immediate encoding if it exists
  //        bool hasImmediate = false;
  //        for (const auto &field : instr.second->getFields()) {
  //          if (field->fieldType() == "imm") {
  //            hasImmediate = true;
  //            break;
  //          }
  //        }
  //        if (hasImmediate) {
  //          ui->encodingTable->setRowHidden(instr.first + 1, true);
  //        }
  //      }
  //    }
  //  }
}

void SliderulesTab::processorChanged() { updateISASelector(); }

void SliderulesTab::updateISASelector(bool forceUpdate) {
  if (auto isaId = ProcessorHandler::currentISA()->isaID();
      forceUpdate || isaId != m_selectedISA) {
    m_selectedISA = isaId;

    emit ui->isaSelector->currentIndexChanged(
        static_cast<int>(ISAFamilies.at(m_selectedISA)));
  }
}

void SliderulesTab::updateRegWidthSelector() {
  ui->regWidthSelector->clear();
  bool isaChanged = false;
  for (const auto &name : ISANames) {
    if (static_cast<int>(ISAFamilies.at(name.first)) ==
        ui->isaSelector->currentIndex()) {
      ui->regWidthSelector->addItem(name.second);
      if (!isaChanged) {
        m_selectedISA = name.first;
        isaChanged = true;
      }
    }
  }

  auto isa = m_encodingTable->model->isa;
  ui->mainExtensionSelector->clear();
  ui->mainExtensionSelector->addItem(isa->baseExtension());
  for (const auto &ext : isa->supportedExtensions()) {
    ui->mainExtensionSelector->addItem(ext);
  }

  emit ui->regWidthSelector->currentIndexChanged(
      static_cast<int>(m_selectedISA));
}

SliderulesTab::~SliderulesTab() { delete ui; }

EncodingModel::EncodingModel(
    const std::shared_ptr<const ISAInfoBase> isa,
    const std::shared_ptr<const InstrVec> instructions,
    const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
    QObject *parent)
    : QAbstractTableModel(parent), isa(isa), instructions(instructions),
      pseudoInstructions(pseudoInstructions) {}

int EncodingModel::rowCount(const QModelIndex &) const {
  return instructions->size();
}
int EncodingModel::columnCount(const QModelIndex &) const { return BIT_END; }

QVariant EncodingModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
  // Hide vertical header
  if (orientation != Qt::Horizontal) {
    return QVariant();
  }

  switch (role) {
  case Qt::DisplayRole: {
    switch (section) {
    case EXTENSION:
      return QString("Extension");
    case TYPE:
      return QString("Type");
    case DESCRIPTION:
      return QString("Description");
    case OPCODE:
      return QString("Opcode");
    case FIELD0:
    case FIELD1:
    case FIELD2:
      return QString("Field ") + QString::number(section - FIELD0);
    }

    if (section >= BIT_START && section < BIT_END) {
      return QString::number(BIT_END - section - 1);
    }
  }
  }

  return QVariant();
}

QVariant EncodingModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() ||
      static_cast<size_t>(index.row()) >= instructions->size()) {
    return QVariant();
  }

  auto row = index.row();
  auto col = index.column();

  if (col > BIT_END) {
    return QVariant();
  }

  // Check if this row contains an immediate field encoding
  auto immIdxIter = m_immediateRows.find(row);
  bool isImmediateRow = immIdxIter != m_immediateRows.end();

  if (isImmediateRow) {
    // This row contains an immediate field encoding.
    // TODO(raccog): Show encoding for constructed immediate here
  } else {
    // This row contains an instruction encoding.

    // Count how many rows before this one contain an immediate field encoding.
    auto prevImmediateIter = m_immediateRows.lower_bound(row);
    int prevImmediateRows =
        prevImmediateIter != m_immediateRows.end()
            ? std::distance(m_immediateRows.begin(), prevImmediateIter)
            : 0;

    auto &instr = instructions->at(row - prevImmediateRows);
    auto fields = instr->getFields();
    switch (role) {
    case Qt::DisplayRole: {
      switch (col) {
      case EXTENSION:
        return QString(instr->extensionOrigin());
      case TYPE:
        // TODO(raccog): Include an instruction's type in isainfo
        return "TODO";
      case DESCRIPTION:
        return QString(instr->description());
      case OPCODE:
        return QString(instr->name());
      case FIELD0:
      case FIELD1:
      case FIELD2:
        if (fields.size() >= static_cast<size_t>(col - FIELD0) + 1)
          return QString(fields.at(col - FIELD0)->fieldType());
        break;
      }

      if (col >= BIT_START && col < BIT_END) {
        unsigned bit = static_cast<unsigned>(BIT_END - col - 1);
        // Check if bit is used in a field.
        for (const auto &field : instr->getFields()) {
          for (const auto &range : field->ranges) {
            if (range.stop == bit) {
              return field->fieldType();
            }
          }
        }
        // Check if bit is used in an opcode part.
        for (const auto &opPart : instr->getOpParts()) {
          if (opPart.range.isWithinRange(bit)) {
            return QString(opPart.bitIsSet(bit) ? "1" : "0");
          }
        }
      }
      break;
    }
    }
  }

  switch (role) {
  case Qt::TextAlignmentRole:
    return Qt::AlignCenter;
  }

  return QVariant();
}

EncodingView::EncodingView(QComboBox &isaFamilySelector, QWidget *parent)
    : QTableView(parent), m_isaFamilySelector(isaFamilySelector) {
  // Add all supported ISA families into the ISA family selector.
  for (const auto &isaFamily : ISAFamilyNames) {
    m_isaFamilySelector.addItem(isaFamily.second);
  }

  auto isa = ProcessorHandler::fullISA();
  updateModel(isa);
  updateView();
}

void EncodingView::updateISA(const QString &isaName,
                             const QStringList &extensions) {
  // FIX(raccog): The name matching does not work, Qt UI uses "RV32",
  // while the ISAInfo class uses "RV32I".
  if (isaName != model->isa->name()) {
    for (const auto &isa : ISANames) {
      if (isa.second == isaName) {
        updateModel(ISAConstructors.at(isa.first)(extensions));
        updateView();
        return;
      }
    }
  }
}

void EncodingView::updateModel(std::shared_ptr<const ISAInfoBase> isa) {
  if (!model || !model->isa->eq(isa.get(), isa->enabledExtensions())) {
    model = std::make_unique<EncodingModel>(
        isa, std::make_shared<const InstrVec>(isa->instructions()),
        std::make_shared<const PseudoInstrVec>(isa->pseudoInstructions()));
    setModel(model.get());
  }
}

void EncodingView::updateView() {
  verticalHeader()->setVisible(false);
  horizontalHeader()->setMinimumSectionSize(30);
  resizeColumnsToContents();
  horizontalHeader()->setSectionResizeMode(EncodingModel::DESCRIPTION,
                                           QHeaderView::ResizeMode::Stretch);
  clearSpans();
  int row = 0;
  for (const auto &instr : *model->instructions) {
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
