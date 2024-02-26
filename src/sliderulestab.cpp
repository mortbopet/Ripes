#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"

namespace Ripes {

ISAEncodingTableModel::ISAEncodingTableModel(QObject *parent) {
  tmp << "RISC-V";
}

int ISAEncodingTableModel::rowCount(const QModelIndex &) const {
  return tmp.size();
}

int ISAEncodingTableModel::columnCount(const QModelIndex &) const { return 1; }

QVariant ISAEncodingTableModel::data(const QModelIndex &index, int role) const {
  if (tmp.size() <= index.row()) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return tmp.at(index.row());
  }

  return QVariant();
}

void ISAEncodingTableModel::change(const QString &s) {
  beginInsertRows(QModelIndex(), tmp.size(), tmp.size());
  tmp << s;
  endInsertRows();
}

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab) {
  ui->setupUi(this);

  isaFamilyBox = ui->isaFamilyBox;
  isaBox = ui->isaBox;
  mainExtBox = ui->mainExtBox;

  for (const auto &familyName : ISAFamilyNames) {
    isaFamilyBox->addItem(familyName.second,
                          QVariant(static_cast<int>(familyName.first)));
  }

  m_encodingModel = std::make_unique<ISAEncodingTableModel>();
  ui->encodingTable->setModel(m_encodingModel.get());

  connect(isaFamilyBox, &QComboBox::activated, this, [=](int index) {
    m_encodingModel->change(isaFamilyBox->itemText(index));
  });
}

SliderulesTab::~SliderulesTab() { delete ui; }

void SliderulesTab::initializeView(const ISAInfoBase &isaInfo) {
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

void SliderulesTab::updateView(const ISAInfoBase &isaInfo,
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

} // namespace Ripes
