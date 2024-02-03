#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"
#include "sliderulesmodels.h"

namespace Ripes {

ISAEncodingFilters::ISAEncodingFilters(QWidget *parent) : QFrame(parent) {
  setLayout(new QHBoxLayout(this));

  auto *isaFamilyFilter = new QFrame(this);
  isaFamilyFilter->setLayout(new QVBoxLayout(isaFamilyFilter));
  isaFamilyFilter->layout()->addWidget(new QLabel("ISA Family"));
  isaFamilyBox = new QComboBox();
  for (const auto &familyName : ISAFamilyNames) {
    isaFamilyBox->addItem(familyName.second,
                          QVariant(static_cast<int>(familyName.first)));
  }
  isaFamilyFilter->layout()->addWidget(isaFamilyBox);
  layout()->addWidget(isaFamilyFilter);
  connect(isaFamilyBox, &QComboBox::activated, this, [=] {
    emit isaFamilyChanged(
        static_cast<ISAFamily>(isaFamilyBox->currentData().toInt()));
  });

  auto *isaFilter = new QFrame(this);
  isaFilter->setLayout(new QVBoxLayout(isaFilter));
  isaFilter->layout()->addWidget(new QLabel("ISA"));
  isaBox = new QComboBox();
  isaFilter->layout()->addWidget(isaBox);
  layout()->addWidget(isaFilter);

  auto *mainExtFilter = new QFrame(this);
  mainExtFilter->setLayout(new QVBoxLayout(mainExtFilter));
  mainExtFilter->layout()->addWidget(new QLabel("Main Extension"));
  mainExtBox = new QComboBox();
  mainExtFilter->layout()->addWidget(mainExtBox);
  layout()->addWidget(mainExtFilter);
}

void ISAEncodingFilters::initializeView(const ISAInfoBase &isaInfo) {
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
}

void ISAEncodingFilters::updateView(const ISAInfoBase &isaInfo,
                                    const ISAInfoBase &prevISAInfo) {
  if (isaInfo.isaFamily() != prevISAInfo.isaFamily()) {
    isaFamilyBox->setCurrentIndex(isaFamilyBox->findData(
        QVariant(static_cast<int>(isaInfo.isaFamily()))));

    isaBox->clear();
    for (const auto &isa : ISAFamilySets.at(isaInfo.isaFamily())) {
      isaBox->addItem(ISANames.at(isa), QVariant(static_cast<int>(isa)));
    }
  }

  if (isaInfo.isaID() != prevISAInfo.isaID()) {
    isaBox->setCurrentIndex(
        isaBox->findData(QVariant(static_cast<int>(isaInfo.isaID()))));
  }

  mainExtBox->clear();
  for (const auto &ext :
       QStringList(isaInfo.baseExtension()) + isaInfo.supportedExtensions()) {
    mainExtBox->addItem(ext, ext);
  }
}

ISAEncodingTableView::ISAEncodingTableView(QWidget *parent)
    : QTableView(parent) {}

void ISAEncodingTableView::updateView(const ISAInfoBase &isaInfo,
                                      const ISAInfoBase &prevISAInfo) {}

void ISAEncodingTableView::initializeView(const ISAInfoBase &isaInfo) {}

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab) {
  ui->setupUi(this);

  m_encodingModel = std::make_unique<ISAEncodingTableModel>();
  auto setISAFromProcessor = [=] {
    const auto *currentISA = ProcessorHandler::currentISA();
    auto isaInfo = ISAInfoRegistry::getISA(currentISA->isaID(),
                                           currentISA->enabledExtensions());
    m_encodingModel->setISAInfo(isaInfo);
  };

  // Initialize the views when the model is initialized
  connect(m_encodingModel.get(), &ISAEncodingTableModel::isaInfoInitialized,
          this, &SliderulesTab::initializeView);

  // Update the views when the model changes
  connect(m_encodingModel.get(), &ISAEncodingTableModel::isaInfoChanged, this,
          &SliderulesTab::updateView);

  // Update the model when the processor is changed
  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged,
          m_encodingModel.get(), setISAFromProcessor);

  // Update the model when the user changes the filters
  connect(ui->encodingFilters, &ISAEncodingFilters::isaFamilyChanged,
          m_encodingModel.get(), &ISAEncodingTableModel::setISAFamily);

  // Initialize model
  setISAFromProcessor();

  ui->encodingTable->setModel(m_encodingModel.get());
}

SliderulesTab::~SliderulesTab() { delete ui; }

void SliderulesTab::initializeView(const ISAInfoBase &isaInfo) {
  ui->encodingFilters->initializeView(isaInfo);
  ui->encodingTable->initializeView(isaInfo);
}

void SliderulesTab::updateView(const ISAInfoBase &isaInfo,
                               const ISAInfoBase &prevISAInfo) {
  ui->encodingFilters->updateView(isaInfo, prevISAInfo);
  ui->encodingTable->updateView(isaInfo, prevISAInfo);
}

} // namespace Ripes
