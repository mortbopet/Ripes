#include "sliderulestab.h"
#include "ui_sliderulestab.h"

#include "processorhandler.h"

namespace Ripes {

ISAInstructionsModel::ISAInstructionsModel(QObject *parent) {
  changeISAInfo(*ProcessorHandler::currentISA());
}

int ISAInstructionsModel::rowCount(const QModelIndex &) const {
  if (!m_isaInfo) {
    return 0;
  }

  return m_isaInfo->instructions().size();
}

int ISAInstructionsModel::columnCount(const QModelIndex &) const { return 2; }

QVariant ISAInstructionsModel::data(const QModelIndex &index, int role) const {
  if (!m_isaInfo || index.row() >= rowCount(QModelIndex())) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return m_isaInfo->instructions().at(index.row())->name();
  }

  return QVariant();
}

QVariant ISAInstructionsModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  return {};
}

const ISAInfoBase *ISAInstructionsModel::isaInfo() const {
  return m_isaInfo.get();
}

const ISAInfoBase *ISAInstructionsModel::prevISAInfo() const {
  return m_prevIsaInfo.get();
}

void ISAInstructionsModel::changeISAFamily(ISAFamily isaFamily) {
  if (!m_isaInfo || m_isaInfo->isaFamily() != isaFamily) {
    changeISA(*ISAFamilySets.at(isaFamily).begin());
  }
}

void ISAInstructionsModel::changeISA(ISA isa) {
  if (!m_isaInfo || m_isaInfo->isaID() != isa) {
    beginResetModel();
    m_prevIsaInfo = m_isaInfo;
    m_isaInfo = ISAInfoRegistry::getISA(isa, QStringList());
    modelChanged();
  }
}

void ISAInstructionsModel::changeISAInfo(const ISAInfoBase &isaInfo) {
  if (!m_isaInfo || m_isaInfo->eq(&isaInfo, isaInfo.enabledExtensions())) {
    beginResetModel();
    m_prevIsaInfo = m_isaInfo;
    m_isaInfo = ISAInfoRegistry::getISA(isaInfo.isaID(), QStringList());
    modelChanged();
  }
}

void ISAInstructionsModel::modelChanged() {
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

  m_encodingModel = std::make_unique<ISAInstructionsModel>();
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

  connect(m_encodingModel.get(), &ISAInstructionsModel::isaInfoChanged, this,
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
