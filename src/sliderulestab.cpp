#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"
#include "sliderulesmodels.h"

namespace Ripes {

ISAFamilySelector::ISAFamilySelector(QWidget *parent) : QComboBox(parent) {
  for (const auto &p : ISAFamilyNames) {
    addItem(p.second, QVariant::fromValue(p.first));
  }
}

ISAFamily ISAFamilySelector::currentFamily() const {
  return QVariant::fromValue(currentData()).value<ISAFamily>();
}

void ISAFamilySelector::setFamily(ISAFamily family) {
  setCurrentIndex(findData(QVariant::fromValue(family)));
  emit familyChanged(family);
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

  // UI -> Model signals
  connect(ui->isaFamilySelector, &ISAFamilySelector::familyActivated,
          m_encodingModel.get(), &ISAEncodingTableModel::setFamily);

  // Model -> UI signals
  connect(m_encodingModel.get(), &ISAEncodingTableModel::familyChanged,
          ui->encodingTable, &ISAEncodingTableView::setFamily);
  connect(m_encodingModel.get(), &ISAEncodingTableModel::familyChanged,
          ui->isaFamilySelector, &ISAFamilySelector::setFamily);

  // Processor changed signal
  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged,
          m_encodingModel.get(), [=] {
            m_encodingModel->setFamily(
                ProcessorHandler::currentISA()->isaFamily());
          });
}

SliderulesTab::~SliderulesTab() { delete ui; }
} // namespace Ripes
