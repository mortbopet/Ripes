#include "registerinitializationwidget.h"
#include "ui_registerinitializationwidget.h"

#include "radix.h"

#include <QComboBox>

namespace Ripes {

std::map<ProcessorID, RegisterInitialization>
    RegisterInitializationWidget::m_initializations;

RegisterSelectionComboBox::RegisterSelectionComboBox(
    RegisterInitializationWidget *parent)
    : QComboBox(parent), m_parent(parent) {
  connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this] {
            const int oldIdx = m_index;
            m_index = currentData().toUInt();
            emit regIndexChanged(oldIdx, m_index);
          });
}

void RegisterSelectionComboBox::showPopup() {
  if (count())
    clear();

  const auto &procisa = ProcessorRegistry::getAvailableProcessors()
                            .at(m_parent->m_currentID)
                            ->isaInfo();
  const auto *isa = procisa.isa.get();
  const auto &initializations =
      m_parent->m_initializations.at(m_parent->m_currentID);

  std::map<std::string_view, std::set<unsigned>> regOptions;
  for (const auto &regFile : isa->regInfos()) {
    for (unsigned i = 0; i < regFile->regCnt(); ++i) {
      if (!regFile->regIsReadOnly(i)) {
        if (regOptions.count(regFile->regFileName()) == 0) {
          regOptions[regFile->regFileName()] = {i};
        } else {
          regOptions[regFile->regFileName()].insert(i);
        }
      }
    }
  }

  for (const auto &regFileInit : initializations) {
    if (regOptions.count(regFileInit.first) > 0) {
      for (const auto &regInit : regFileInit.second) {
        regOptions.at(regFileInit.first).erase(regInit.first);
      }
    }
  }

  for (const auto &regFileInit : regOptions) {
    for (const auto &i : regFileInit.second) {
      if (auto opt = isa->regInfo(regFileInit.first)) {
        auto regInfo = opt.value();
        addItem(regInfo->regName(i) + " (" + regInfo->regAlias(i) + ")", i);
      }
    }
  }
  QComboBox::showPopup();
}

RegisterInitializationWidget::RegisterInitializationWidget(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::RegisterInitializationWidget) {
  m_ui->setupUi(this);

  m_hexValidator = new QRegularExpressionValidator(this);
  m_hexValidator->setRegularExpression(hexRegex32);

  const QIcon addIcon = QIcon(":/icons/plus.svg");
  m_ui->addInitButton->setIcon(addIcon);
  connect(m_ui->addInitButton, &QPushButton::clicked, this, [this] {
    if (auto regIdx = getNonInitializedRegIdx(); regIdx.has_value())
      this->addRegisterInitialization(regIdx->file->regFileName(),
                                      regIdx->index);
  });

  // Initialize initializations for all available processors
  for (const auto &desc : ProcessorRegistry::getAvailableProcessors()) {
    // Set to initial value if this is the first time the dialog is loaded.
    // Else, keep whatever changes has been stored in the static variable, made
    // in previous invokations of the dialog.
    if (!m_initializations.count(desc.second->id)) {
      m_initializations[desc.second->id] = desc.second->defaultRegisterVals;
    }
  }
}

void RegisterInitializationWidget::processorSelectionChanged(ProcessorID id) {
  // Clear current initialization widgets and recreate initialization widgets
  // based on the currently selected processor
  m_currentRegInitWidgets.clear();
  m_currentID = id;

  for (const auto &regFileInit : m_initializations.at(id)) {
    for (const auto &regInit : regFileInit.second) {
      addRegisterInitialization(regFileInit.first, regInit.first);
    }
  }

  updateAddButtonState();
}

RegisterInitializationWidget::~RegisterInitializationWidget() { delete m_ui; }

void RegisterInitializationWidget::updateAddButtonState() {
  // Disable add button if we have exhausted the number of registers to
  // initialize for the given processor
  if (getNonInitializedRegIdx().has_value()) {
    m_ui->addInitButton->setEnabled(true);
  } else {
    m_ui->addInitButton->setEnabled(false);
  }
}

std::optional<RegIndex>
RegisterInitializationWidget::getNonInitializedRegIdx() {
  const auto &currentISA =
      ProcessorRegistry::getAvailableProcessors().at(m_currentID)->isaInfo();
  const auto *isa = currentISA.isa.get();
  const auto &currentInitForProc = m_initializations.at(m_currentID);
  for (const auto &regFileInit : currentInitForProc) {
    unsigned id = 0;
    if (auto opt = isa->regInfo(regFileInit.first)) {
      auto regInfo = opt.value();
      while (regFileInit.second.count(id) || regInfo->regIsReadOnly(id)) {
        id++;
      }
      if (id < regInfo->regCnt()) {
        return RegIndex{isa->regInfoMap().at(regFileInit.first), id};
      }
    }
  }

  return {};
}

void RegisterInitializationWidget::addRegisterInitialization(
    const std::string_view &regFileName, unsigned regIdx) {
  constexpr unsigned s_defaultval = 0;
  const auto &procisa =
      ProcessorRegistry::getAvailableProcessors().at(m_currentID)->isaInfo();
  const auto *isa = procisa.isa.get();
  auto maybeRegInfo = isa->regInfo(regFileName);
  if (!maybeRegInfo.has_value()) {
    return;
  }
  auto regInfo = *maybeRegInfo;

  if (!m_initializations.at(m_currentID).count(regFileName)) {
    m_initializations.at(m_currentID)[regFileName] = {};
  }
  if (!m_initializations.at(m_currentID).at(regFileName).count(regIdx)) {
    // No default value of the register initialization exists.
    m_initializations.at(m_currentID).at(regFileName)[regIdx] = s_defaultval;
  }

  const auto &regLayout = m_ui->regInitLayout;

  auto &w =
      m_currentRegInitWidgets.emplace_back(std::make_unique<RegInitWidgets>());
  auto *w_ptr = w.get();

  w->regFileName = regFileName;
  w->name = new RegisterSelectionComboBox(this);
  w->value = new QLineEdit(this);
  w->remove = new QPushButton(this);

  w->name->addItem(regInfo->regName(regIdx) + " (" + regInfo->regAlias(regIdx) +
                       ")",
                   regIdx);

  connect(w->name, &RegisterSelectionComboBox::regIndexChanged, this,
          [this, w_ptr, regFileName](int oldIdx, int newIdx) {
            m_initializations.at(m_currentID).at(regFileName).erase(oldIdx);
            m_initializations.at(m_currentID).at(regFileName)[newIdx];
            emit w_ptr->value->textChanged(w_ptr->value->text());
          });

  const QIcon removeIcon = QIcon(":/icons/delete.svg");
  w->remove->setIcon(removeIcon);

  w->value->setValidator(m_hexValidator);
  w->value->setText(
      "0x" +
      QString::number(
          m_initializations.at(m_currentID).at(regFileName).at(regIdx), 16));
  connect(w->value, &QLineEdit::textChanged, this,
          [this, w_ptr, regFileName](const QString &text) {
            this->m_initializations.at(this->m_currentID)
                .at(regFileName)
                .at(w_ptr->name->currentData().toUInt()) =
                text.toUInt(nullptr, 16);
          });

  const int nInitializations = regLayout->rowCount();
  regLayout->addWidget(w->name, nInitializations, 0);
  regLayout->addWidget(w->value, nInitializations, 1);
  regLayout->addWidget(w->remove, nInitializations, 2);
  connect(w->remove, &QPushButton::clicked, this,
          [this, w_ptr] { this->removeRegInitWidget(w_ptr); });

  updateAddButtonState();
}

void RegisterInitializationWidget::removeRegInitWidget(
    RegisterInitializationWidget::RegInitWidgets *w) {
  auto iter = std::find_if(
      m_currentRegInitWidgets.begin(), m_currentRegInitWidgets.end(),
      [w](const auto &initWidget) { return initWidget.get() == w; });
  assert(iter != m_currentRegInitWidgets.end());

  // Current register index stored in the combobox of the regInitWidgets
  const unsigned regIdx = w->name->itemData(0).toUInt();
  m_initializations.at(m_currentID).at(w->regFileName).erase(regIdx);

  m_currentRegInitWidgets.erase(iter);
  updateAddButtonState();
}

void RegisterInitializationWidget::RegInitWidgets::clear() {
  delete name;
  delete value;
  delete remove;
}

RegisterInitialization RegisterInitializationWidget::getInitialization() const {
  return m_initializations.at(m_currentID);
}

} // namespace Ripes
