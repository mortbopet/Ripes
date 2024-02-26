#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"

namespace Ripes {

ISAInstructionsModel::ISAInstructionsModel(QObject *parent) {}

int ISAInstructionsModel::rowCount(const QModelIndex &) const { return 0; }

int ISAInstructionsModel::columnCount(const QModelIndex &) const { return 0; }

QVariant ISAInstructionsModel::data(const QModelIndex &index, int role) const {
  return {};
}

QVariant ISAInstructionsModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  return {};
}

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab) {
  ui->setupUi(this);

  isaFamilyBox = ui->isaFamilyBox;
  isaBox = ui->isaBox;
  mainExtBox = ui->mainExtBox;
  baseExtCheckBox = ui->baseExtCheckBox;

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
