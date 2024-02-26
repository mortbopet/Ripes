#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"

namespace Ripes {

ISAEncodingTableModel::ISAEncodingTableModel(QObject *parent)
    : m_isaInfo(ProcessorHandler::fullISA()) {}

int ISAEncodingTableModel::rowCount(const QModelIndex &) const {
  if (!m_isaInfo) {
    return 0;
  }

  return m_isaInfo->instructions().size();
}

int ISAEncodingTableModel::columnCount(const QModelIndex &) const { return 1; }

QVariant ISAEncodingTableModel::data(const QModelIndex &index, int role) const {
  if (!m_isaInfo || index.row() >= rowCount(QModelIndex())) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return m_isaInfo->instructions().at(index.row())->name();
  }

  return QVariant();
}
const ISAInfoBase *ISAEncodingTableModel::isaInfo() const {
  return m_isaInfo.get();
}

const ISAInfoBase *ISAEncodingTableModel::prevISAInfo() const {
  return m_prevIsaInfo.get();
}

void ISAEncodingTableModel::changeISAFamily(ISAFamily isaFamily) {
  if (!m_isaInfo || m_isaInfo->isaFamily() != isaFamily) {
    changeISA(*ISAFamilySets.at(isaFamily).begin());
  }
}

void ISAEncodingTableModel::changeISA(ISA isa) {
  if (!m_isaInfo || m_isaInfo->isaID() != isa) {
    beginResetModel();
    m_prevIsaInfo = m_isaInfo;
    m_isaInfo = ISAInfoRegistry::getISA(isa, QStringList());
    modelChanged();
  }
}

void ISAEncodingTableModel::changeISAInfo(const ISAInfoBase &isaInfo) {
  if (!m_isaInfo || m_isaInfo->eq(&isaInfo, isaInfo.enabledExtensions())) {
    beginResetModel();
    m_prevIsaInfo = m_isaInfo;
    m_isaInfo = ISAInfoRegistry::getISA(isaInfo.isaID(), QStringList());
    modelChanged();
  }
}

void ISAEncodingTableModel::modelChanged() {
  endResetModel();
  emit isaInfoChanged(*m_isaInfo);
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

  m_encodingModel = std::make_unique<ISAEncodingTableModel>();
  ui->encodingTable->setModel(m_encodingModel.get());

  connect(isaFamilyBox, &QComboBox::activated, this, [=](int index) {
    m_encodingModel->changeISAFamily(
        static_cast<ISAFamily>(isaFamilyBox->itemData(index).toInt()));
  });
  connect(isaBox, &QComboBox::activated, this, [=](int index) {
    m_encodingModel->changeISA(
        static_cast<ISA>(isaBox->itemData(index).toInt()));
  });
  connect(mainExtBox, &QComboBox::activated, this, [=](int index) {
    auto ext = mainExtBox->itemData(index).toString();
    const auto *isaInfo = m_encodingModel->isaInfo();
    auto enabledExts = isaInfo->enabledExtensions();
    if (ext != isaInfo->baseExtension() && !enabledExts.contains(ext)) {
      enabledExts << ext;
      m_encodingModel->changeISAInfo(
          *ISAInfoRegistry::getISA(isaInfo->isaID(), enabledExts));
    }
  });

  connect(m_encodingModel.get(), &ISAEncodingTableModel::isaInfoChanged, this,
          &SliderulesTab::updateView);

  updateView(*m_encodingModel->isaInfo());
}

SliderulesTab::~SliderulesTab() { delete ui; }

void SliderulesTab::updateView(const ISAInfoBase &isaInfo) {
  const auto *prevISAInfo = m_encodingModel->prevISAInfo();

  if (!prevISAInfo || isaInfo.isaFamily() != prevISAInfo->isaFamily()) {
    isaFamilyBox->setCurrentIndex(isaFamilyBox->findData(
        QVariant(static_cast<int>(isaInfo.isaFamily()))));

    isaBox->clear();
    for (const auto &isa : ISAFamilySets.at(isaInfo.isaFamily())) {
      isaBox->addItem(ISANames.at(isa), QVariant(static_cast<int>(isa)));
    }
  }

  if (!prevISAInfo || isaInfo.isaID() != prevISAInfo->isaID()) {
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
}

} // namespace Ripes
