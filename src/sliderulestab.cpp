#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"
#include "sliderulesmodels.h"

namespace Ripes {

ISAFamilyComboBox::ISAFamilyComboBox(QWidget *parent) : QComboBox(parent) {
  for (const auto &p : ISAFamilyNames) {
    addItem(p.second, QVariant::fromValue(p.first));
  }
}

void ISAFamilyComboBox::setFamily(ISAFamily family) {
  setCurrentIndex(findData(QVariant::fromValue(family)));
  emit familyChanged(family);
}

ISAComboBox::ISAComboBox(QWidget *parent) : QComboBox(parent) {}

void ISAComboBox::setFamily(ISAFamily family) {
  ISA prevISA = currentData().value<ISA>();
  clear();
  for (const auto isa : ISAFamilySets.at(family)) {
    addItem(ISANames.at(isa), QVariant::fromValue(isa));
  }
  ISA isa = currentData().value<ISA>();
  if (prevISA != isa) {
    emit isaChanged(isa);
  }
}

void ISAComboBox::setISA(ISA isa) {
  setCurrentIndex(findData(QVariant::fromValue(isa)));
  emit isaChanged(isa);
}

ISAEncodingTableView::ISAEncodingTableView(QWidget *parent)
    : QTableView(parent) {}

void ISAEncodingTableView::setFamily(ISAFamily family) {
  emit familyChanged(family);
}

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab) {
  ui->setupUi(this);

  // Set initial model
  m_encodingModel = std::make_unique<ISAEncodingTableModel>();
  ui->encodingTable->setModel(m_encodingModel.get());

  // Helper signals
  connect(ui->isaFamilySelector, &QComboBox::activated, ui->isaFamilySelector,
          [=](int index) {
            emit ui->isaFamilySelector->familyActivated(
                static_cast<ISAFamily>(index));
          });
  connect(ui->isaSelector, &QComboBox::activated, ui->isaSelector,
          [=](int index) {
            emit ui->isaSelector->isaActivated(static_cast<ISA>(index));
          });

  // UI -> Model signals
  connect(ui->isaFamilySelector, &ISAFamilyComboBox::familyActivated,
          m_encodingModel.get(), &ISAEncodingTableModel::setFamily);
  connect(ui->isaSelector, &ISAComboBox::isaActivated, m_encodingModel.get(),
          &ISAEncodingTableModel::setISA);

  // Model -> UI signals
  connect(m_encodingModel.get(), &ISAEncodingTableModel::familyChanged,
          ui->encodingTable, &ISAEncodingTableView::setFamily);
  connect(m_encodingModel.get(), &ISAEncodingTableModel::familyChanged,
          ui->isaFamilySelector, &ISAFamilyComboBox::setFamily);
  connect(m_encodingModel.get(), &ISAEncodingTableModel::familyChanged,
          ui->isaSelector, &ISAComboBox::setFamily);
  connect(m_encodingModel.get(), &ISAEncodingTableModel::isaChanged,
          ui->isaSelector, &ISAComboBox::setISA);

  // Model data changed
  connect(m_encodingModel.get(), &ISAEncodingTableModel::familyChanged,
          m_encodingModel.get(),
          [=] { emit m_encodingModel->layoutChanged(); });
  connect(m_encodingModel.get(), &ISAEncodingTableModel::isaChanged,
          m_encodingModel.get(),
          [=] { emit m_encodingModel->layoutChanged(); });

  // Processor changed signal
  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged,
          m_encodingModel.get(), [=] {
            m_encodingModel->setFamily(
                ProcessorHandler::currentISA()->isaFamily());
          });

  m_encodingModel->setFamily(ProcessorHandler::currentISA()->isaFamily());
}

SliderulesTab::~SliderulesTab() { delete ui; }
} // namespace Ripes
