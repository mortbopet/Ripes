#include "sliderulestab.h"
#include "processorhandler.h"
#include "qcheckbox.h"
#include "ui_sliderulestab.h"

#include "rv_i_ext.h"

namespace Ripes {

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    // TODO(raccog): Initialize default selected ISA using cached settings
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab) {
  ui->setupUi(this);

  // Create encoding table and transfer ownership to UI layout.
  m_encodingTable = new EncodingView(*ui->isaSelector, *ui->regWidthSelector,
                                     *ui->mainExtensionSelector,
                                     *ui->extensionCheckboxGrid, *ui->title);
  ui->encodingLayout->addWidget(m_encodingTable);
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

EncodingView::EncodingView(QComboBox &isaFamilySelector,
                           QComboBox &regWidthSelector,
                           QComboBox &mainExtensionSelector,
                           QGridLayout &additionalExtensionSelectors,
                           QLabel &title, QWidget *parent)
    : QTableView(parent), m_isaFamilySelector(isaFamilySelector),
      m_regWidthSelector(regWidthSelector),
      m_mainExtensionSelector(mainExtensionSelector),
      m_additionalExtensionSelectors(additionalExtensionSelectors),
      m_title(title) {
  // Add all supported ISA families into the ISA family selector.
  for (const auto &isaFamily : ISAFamilyNames) {
    m_isaFamilySelector.addItem(isaFamily.second,
                                QVariant(static_cast<int>(isaFamily.first)));
  }

  connect(this, &EncodingView::modelUpdated, this, &EncodingView::updateView);

  connect(&m_isaFamilySelector, &QComboBox::activated, this,
          &EncodingView::isaFamilyChanged);
  connect(&m_regWidthSelector, &QComboBox::activated, this,
          &EncodingView::regWidthChanged);
  connect(&m_mainExtensionSelector, &QComboBox::activated, this,
          &EncodingView::mainExtensionChanged);

  processorChanged();

  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this,
          &EncodingView::processorChanged);
}

void EncodingView::processorChanged() {
  auto isaInfo = ProcessorHandler::currentISA();
  updateModel(
      ISAInfoRegistry::getISA(isaInfo->isaID(), isaInfo->enabledExtensions()));
}

void EncodingView::isaFamilyChanged(int index) {
  if (index < 0) {
    return;
  }

  ISAFamily isaFamily = static_cast<ISAFamily>(index);
  ISA isa = *ISAFamilySets.at(isaFamily).begin();
  auto isaInfo = ISAInfoRegistry::getISA(isa, QStringList());
  updateModel(isaInfo);
}

void EncodingView::regWidthChanged(int index) {
  if (index < 0) {
    return;
  }

  bool ok;
  auto isaInfo = ISAInfoRegistry::getISA(
      static_cast<ISA>(m_regWidthSelector.itemData(index).toInt(&ok)),
      QStringList());
  assert(ok && "ISA register width filter has an invalid ISA QVariant");
  updateModel(isaInfo);
}

void EncodingView::mainExtensionChanged(int index) {
  if (index < 0) {
    return;
  }

  auto selectedExtension = m_mainExtensionSelector.currentText();
  auto extensions = m_model->isa->baseExtension() != selectedExtension
                        ? QStringList(selectedExtension)
                        : QStringList();
  auto isa = ISAInfoRegistry::getISA(m_model->isa->isaID(), extensions);
  if (m_model->isa->eq(isa.get(), isa->enabledExtensions())) {
    updateView(*isa.get());
  } else {
    updateModel(isa);
  }
}

void EncodingView::updateModel(std::shared_ptr<const ISAInfoBase> isa) {
  if (!m_model || !m_model->isa->eq(isa.get(), isa->enabledExtensions())) {
    auto model = std::make_unique<EncodingModel>(
        isa, std::make_shared<const InstrVec>(isa->instructions()),
        std::make_shared<const PseudoInstrVec>(isa->pseudoInstructions()));
    setModel(model.get());
    emit modelUpdated(*model->isa.get());
    m_model.swap(model);
  }
}

void EncodingView::updateView(const ISAInfoBase &isaInfo) {
  // Note that `isaInfo` is the updated ISA, while `m_model->isa` contains
  // the previous ISA. This can be used to compare differences between the two
  // to see what UI needs to be updated.
  //
  // Further note that `m_model` will be NULL the first time this function is
  // called. Ensure that it is only dereferenced if it is known to not be NULL.

  ISAFamily isaFamily = ISAFamilies.at(isaInfo.isaID());
  int familyIdx = static_cast<int>(isaFamily);

  if (!m_model || m_model->isa->isaID() != isaInfo.isaID()) {
    // Setup main extension filter
    m_mainExtensionSelector.clear();
    m_mainExtensionSelector.addItem(isaInfo.baseExtension());
    m_mainExtensionSelector.addItems(isaInfo.supportedExtensions());

    // Setup additional extension filters
    QLayoutItem *item;
    while ((item = m_additionalExtensionSelectors.takeAt(0)) != nullptr) {
      QWidget *widget = item->widget();
      if (widget) {
        delete widget;
      }
      delete item;
    }
    auto baseCheckbox = new QCheckBox(isaInfo.baseExtension());
    m_additionalExtensionSelectors.addWidget(baseCheckbox, 0, 0);
    int extCol = 1;
    int extRow = 0;
    const int MAX_COL = 3;
    for (const auto &extName : isaInfo.supportedExtensions()) {
      auto extCheckbox = new QCheckBox(extName);
      m_additionalExtensionSelectors.addWidget(extCheckbox, extRow, extCol);
      ++extCol;
      if (extCol >= MAX_COL) {
        extCol = 0;
        ++extRow;
      }
    }
  }

  // Update checkboxes to display the current ISA's extensions
  for (int i = 0; i < m_additionalExtensionSelectors.count(); ++i) {
    const auto &checkbox = reinterpret_cast<QCheckBox *>(
        m_additionalExtensionSelectors.itemAt(i)->widget());
    if (m_mainExtensionSelector.currentText() == checkbox->text()) {
      checkbox->setChecked(true);
      checkbox->setEnabled(false);
    } else {
      checkbox->setEnabled(true);
      checkbox->setChecked(isaInfo.baseExtension() == checkbox->text() ||
                           isaInfo.extensionEnabled(checkbox->text()));
    }
  }

  // Setup ISA family and register width selectors
  if (m_regWidthSelector.count() == 0 || !m_model ||
      m_model->isa->isaFamily() != isaFamily) {
    m_regWidthSelector.clear();
    int regWidthIdx = 0;
    for (auto isa : ISAFamilySets.at(isaFamily)) {
      auto isaName = ISANames.at(isa);
      m_regWidthSelector.addItem(isaName, QVariant(static_cast<int>(isa)));
      if (isaInfo.isaID() == isa) {
        m_regWidthSelector.setCurrentIndex(regWidthIdx);
      }
      ++regWidthIdx;
    }

    m_isaFamilySelector.setCurrentIndex(familyIdx);
  }

  // Setup spans for fields and opcodes
  clearSpans();
  int row = 0;
  const auto &mainSelectedExtension = m_mainExtensionSelector.currentText();
  auto filteredInstructions = 0;
  for (const auto &instr : isaInfo.instructions()) {
    setRowHidden(row, false);
    for (const auto &field : instr->getFields()) {
      for (const auto &range : field->ranges) {
        if (range.width() > 1) {
          setSpan(row, EncodingModel::BIT_END - range.stop - 1, 1,
                  range.width());
        }
      }
    }
    // Hide instructions that are filtered out
    if (mainSelectedExtension != isaInfo.baseExtension() &&
        isaInfo.baseExtension() == instr->extensionOrigin()) {
      setRowHidden(row, true);
      ++filteredInstructions;
    }
    ++row;
  }

  // Update title with table details
  m_title.setText(
      QString("Encoding Table (%1 instructions shown %2 instructions filtered)")
          .arg(QString::number(isaInfo.instructions().size() -
                               filteredInstructions),
               QString::number(filteredInstructions)));

  // Encoding table properties
  verticalHeader()->setVisible(false);
  horizontalHeader()->setMinimumSectionSize(30);
  resizeColumnsToContents();
  horizontalHeader()->setSectionResizeMode(EncodingModel::DESCRIPTION,
                                           QHeaderView::ResizeMode::Stretch);
}

DecodingModel::DecodingModel(const std::shared_ptr<const ISAInfoBase> isa,
                             const std::shared_ptr<const InstrVec> instructions,
                             QObject *parent)
    : QAbstractTableModel(parent), m_isa(isa), m_instructions(instructions) {}

int DecodingModel::rowCount(const QModelIndex &) const { return 0; }
int DecodingModel::columnCount(const QModelIndex &) const { return 0; }
QVariant DecodingModel::data(const QModelIndex &, int) const {
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
