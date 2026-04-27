#include "processorselectiondialog.h"
#include "ui_processorselectiondialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>

#include "processorhandler.h"
#include "radix.h"
#include "ripessettings.h"

namespace Ripes {

#define BIT_WIDTH_SUFFIX "-Bit"


VariationInfo::Options_t VariationSet::extractOptions() const {
  VariationInfo::Options_t options;

  for (const auto *variation : variations) {
    options.insert(variation->options.begin(), variation->options.end());
  }

  return options;
}
ExtensionContainer_t VariationSet::extractExtensions() const {
  ExtensionContainer_t extensions;
  
  for (const auto *variation : variations) {
    const ExtensionContainer_t& exts = procDesc.variations.at(variation->id)->isaInfo().supportedExtensions->extensions();
    extensions.insert(exts.begin(), exts.end());
  }

  return extensions;
}

VariationSet& VariationSet::filter(ExtensionSetInfo::ConstPtr extensions) {
  for (auto it = this->begin(); it != this->end(); ) {
    Q_ASSERT(procDesc.hasVariation((*it)->id) && 
      "Internal Sanity Check: Variation must exist in processor class info variations map.");
    
    // variation must be compatible with the selected extensions
    const auto supportedExt = procDesc.variations.at((*it)->id)->isaInfo().supportedExtensions;
    if ( !extensions->isSubsetOf(*supportedExt) ) {
      it = this->erase(it);
    } else {
      ++it;
    }
  }

  return *this;
}
VariationSet& VariationSet::filter(const VariationInfo::Options_t& options, bool matchExact) {
  for (auto it = this->begin(); it != this->end(); ) {
    Q_ASSERT(procDesc.hasVariation((*it)->id) && 
      "Internal Sanity Check: Variation must exist in processor class info variations map.");

    // variation must contain at least the selected options
    for (const auto &opt : options) {
      if ( !(*it)->options.count(opt) ) {
        it = this->erase(it);
        goto next_iteration;
      }
    }

    if (matchExact) {
      // variation must not contain options that are not selected
      for (const auto &opt : (*it)->options) {
        if ( !options.count(opt) ) {
          it = this->erase(it);
          goto next_iteration;
        }
      }
    }

    ++it;
    next_iteration: continue;
  }

  return *this;
}


class ExtensionCheckBox : public QCheckBox {
  Q_OBJECT

public:
  explicit ExtensionCheckBox(const ExtensionInfoInterface* ext, QWidget* parent = nullptr) 
  : QCheckBox(ext->name(), parent), m_ext(ext) {
    setToolTip(ext->description());
  }

  const ExtensionInfoInterface* extension() const { return m_ext; }

private:
  const ExtensionInfoInterface* m_ext;
};


ProcessorSelectionDialog::ProcessorSelectionDialog(QWidget *parent)
    : QDialog(parent), m_ui(new Ui::ProcessorSelectionDialog) {
  m_ui->setupUi(this);
  setWindowTitle("Select Processor");

  // Initialize top level ISA items
  m_ui->processors->setHeaderHidden(true);
  std::map<QString, QTreeWidgetItem *> isaFamilyItems;
  
  for (const auto &isa : ISAFamilyNames) {
    if (isaFamilyItems.count(isa.second) == 0) {
      isaFamilyItems[isa.second] = new QTreeWidgetItem({isa.second});
      auto *isaItem = isaFamilyItems.at(isa.second);
      isaItem->setFlags(isaItem->flags() & ~(Qt::ItemIsSelectable));
      m_ui->processors->insertTopLevelItem(m_ui->processors->topLevelItemCount(),
                                           isaItem);
    }
  }

  // Initialize processor list
  QTreeWidgetItem *selectedItem = nullptr;

  for (auto &desc : ProcessorRegistry::getAvailableProcessors()) {
    QTreeWidgetItem *processorItem = new QTreeWidgetItem({desc.second.name});
    processorItem->setData(ProcessorColumn, Qt::UserRole,
                           QVariant::fromValue(desc.second.id));

    const QString &isaFamily = ISAFamilyNames.at(desc.second.isa);
    QTreeWidgetItem *familyItem = isaFamilyItems.at(isaFamily);

    if (desc.second.id == ProcessorHandler::getID()) {
      // Highlight if currently selected processor
      auto font = processorItem->font(ProcessorColumn);
      font.setBold(true);
      processorItem->setFont(ProcessorColumn, font);
      selectedItem = processorItem;
      familyItem->setExpanded(true);
    }

    familyItem->insertChild(familyItem->childCount(), processorItem);
  }

  // connect signals
  connect(m_ui->processors, &QTreeWidget::currentItemChanged, this,
          &ProcessorSelectionDialog::selectionChanged);

  connect(m_ui->width, &QComboBox::currentTextChanged, this, 
    [this](QString text) {
      m_selectedBitWidth = text.remove(BIT_WIDTH_SUFFIX).toUInt();
      buildSelector();
  });
  
  
  connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);


  if (selectedItem != nullptr) {
    m_ui->processors->setCurrentItem(selectedItem);
    
    m_selectedID = ProcessorHandler::getID();

    const auto& varDesc = ProcessorRegistry::getDescription(m_selectedID, ProcessorHandler::getVariationID());

    m_selectedOptions = varDesc->variationInfo.options;
    m_selectedBitWidth = varDesc->variationInfo.bitWidth;
    
    auto settings = RipesSettings::value(RIPES_SETTING_PROCESSOR_EXTENSIONS);
    if (settings.isNull()) {
      m_selectedExtensions = std::move(varDesc->isaInfo().defaultExtensions->clone());
    } else {
      // remove extensions that are not preselected in the global settings
      m_selectedExtensions = std::move(varDesc->isaInfo().supportedExtensions->clone());
      QList<uint> extensionIDs = settings.value<QList<uint>>();
      for (const auto* ext : varDesc->isaInfo().supportedExtensions->extensions()) {
        if (!extensionIDs.contains(ext->id())) {
          m_selectedExtensions->remove(*ext);
        }
      }
    }

    if (activeVariationIsValid()) {
      // build the selector with the current settings if they are valid
      unsigned layoutID =
        RipesSettings::value(RIPES_SETTING_PROCESSOR_LAYOUT_ID).toInt();
      if (layoutID >= varDesc->layouts.size()) {
        layoutID = 0;
      }
      m_ui->layout->setCurrentIndex(layoutID);
      
      buildSelector();
    } else {
      // otherwise, let the preselected processor be the default initialized
      m_ui->processors->setCurrentItem(selectedItem);
    }
  }
}

RegisterInitialization
ProcessorSelectionDialog::getRegisterInitialization() const {
  return m_ui->regInitWidget->getInitialization();
}

ProcessorSelectionDialog::~ProcessorSelectionDialog() { delete m_ui; }

/// Get the variation ID of the first full matching variation in respect to the currently selected variation info else nullopt
std::optional<VariationID> ProcessorSelectionDialog::getConfiguredVariationId() const {
  const VariationSet possible_variations = getMatchingVariations(true);

  if ( possible_variations.size() == 0 )
    return std::nullopt;

  const ProcClassInfo &procDesc = getSelectedProcessorClass();

  // get the first variation with the smallest number of supported extensions as the "best" matching variation
  const VariationInfo* best_variation = *std::min_element(possible_variations.begin(), possible_variations.end(),
    [&procDesc](const auto* a, const auto* b) {
      const auto& a_ext = procDesc.variations.at(a->id)->isaInfo().supportedExtensions->extensions();
      const auto& b_ext = procDesc.variations.at(b->id)->isaInfo().supportedExtensions->extensions();
      return a_ext.size() < b_ext.size();
    }
  );

  return best_variation->id;
}

/// test that the currently active variation in the set of all variations of the selected processor class
bool ProcessorSelectionDialog::activeVariationIsValid() const {
  return getConfiguredVariationId().has_value();
}


VariationSet ProcessorSelectionDialog::getMatchingVariations(bool matchExact) const {
  VariationSet variations = getAvailableVariations();
  
  variations.filter(m_selectedExtensions);
  variations.filter(m_selectedOptions, matchExact);

  return variations;
}

VariationSet ProcessorSelectionDialog::getAvailableVariations() const {
  const ProcClassInfo &procDesc = getSelectedProcessorClass();
  return VariationSet{
    procDesc,
    procDesc.getVariationsByBitWidth(m_selectedBitWidth)
  };
}


bool ProcessorSelectionDialog::isCPUItem(const QTreeWidgetItem *item) const {
  if (!item) {
    return false;
  }
  QVariant selectedItemData = item->data(ProcessorColumn, Qt::UserRole);
  const bool validSelection = selectedItemData.canConvert<ProcessorID>();
  return validSelection;
}


static void clearLayout(QLayout *layout) {
  if (!layout) {
    return;
  }

  while (QLayoutItem *item = layout->takeAt(0)) {
    if (QLayout *childLayout = item->layout()) {
      clearLayout(childLayout);
    }
    
    if (QWidget *widget = item->widget()) {
      delete widget;
    }

    delete item;
  }
}


void ProcessorSelectionDialog::buildSelector() {
  const ProcClassInfo &desc = getSelectedProcessorClass();

  const VariationSet variations = getAvailableVariations();
  
  //------------------------------------------------------------------------------
  // Build Bit Width selector
  //------------------------------------------------------------------------------
  m_ui->width->blockSignals(true);
  m_ui->width->clear();
  auto bitWidths = desc.getBitWidths();
  
  int bwIndex = 0;
  for (const uint bw : bitWidths) {
    m_ui->width->addItem(QString::number(bw)+BIT_WIDTH_SUFFIX);
    
    if (bw == m_selectedBitWidth) {
      bwIndex = m_ui->width->count() - 1;
    }
  }

  Q_ASSERT( bwIndex != -1 &&
      "Internal error: selected variation bit-width must always be available." );
  m_ui->width->setCurrentIndex(bwIndex);
  m_ui->width->blockSignals(false);

  //------------------------------------------------------------------------------
  // Build Extensions
  //------------------------------------------------------------------------------
  clearLayout(m_ui->extensions);

  std::set<ExtensionID_t> availableExtensions;
  for (const auto &variation : variations) {
    Q_ASSERT(desc.hasVariation(variation->id) && 
      "Internal Sanity Check: Variation must exist in processor class info variations map.");
    const auto &isaInfo = desc.variations.at(variation->id)->isaInfo();
    
    for (const auto *ext : isaInfo.supportedExtensions->extensions()) {
      if (availableExtensions.contains(ext->id())) {
        continue;
      }
      
      availableExtensions.insert(ext->id());
      
      auto chkbox = new ExtensionCheckBox(ext);

      m_ui->extensions->addWidget(chkbox);
      if (m_selectedExtensions->containsExtension(*ext)) {
        chkbox->setChecked(true);
      }
      connect(chkbox, &QCheckBox::toggled, this, [this, ext](bool toggled) {
        if (toggled) {
          *(this->m_selectedExtensions) << *ext;
        } else {
          this->m_selectedExtensions->remove(*ext);
        }
        
        // extensionChanged();
        variationSelectionChanged();
      });
    }
  }
  
  // making sure that preselected extensions that are not available for the selected processor are disregarded
  for (const auto &ext : m_selectedExtensions->extensions()) {
    if (!availableExtensions.contains(ext->id())) {
      m_selectedExtensions->remove(*ext);
    }
  }

  //------------------------------------------------------------------------------
  // Build Options
  //------------------------------------------------------------------------------
  clearLayout(m_ui->options);

  const auto availableOptions = variations.extractOptions();

  for (const auto &opt : availableOptions) {
    auto chkbox = new QCheckBox(opt);
    m_ui->options->addWidget(chkbox);
    
    // activate default options
    if (m_selectedOptions.contains(opt)) {
      chkbox->setChecked(true);
    }

    connect(chkbox, &QCheckBox::toggled, this, [this, opt](bool toggled) {
      if (toggled) {
        this->m_selectedOptions.insert(opt);
      } else {
        this->m_selectedOptions.erase(opt);
      }

      // optionChanged();
        variationSelectionChanged();
    });
  }

  // making sure that preselected options that are not available for the selected processor are disregarded
  for (const auto &opt : m_selectedOptions) {
    if (!availableOptions.contains(opt)) {
      m_selectedOptions.erase(opt);
    }
  }

  //------------------------------------------------------------------------------
  // Build Layout
  //------------------------------------------------------------------------------
  // gets build and configured in propagateVariationChange and only then a valid 
  // variation is selected

  // propagating the preselected extensions so that invalid options are disabled 
  // extensionChanged();

  // propagating the preselected and propagated options so that invalid extensions are disabled
  // optionChanged();

  variationSelectionChanged();
}

void ProcessorSelectionDialog::variationSelectionChanged() {
  VariationSet variations = getMatchingVariations(false);
  
  //------------------------------------------------------------------------------
  // Extension Management
  //------------------------------------------------------------------------------
  const ExtensionContainer_t availableExtensions = variations.extractExtensions();
  for (int i = 0; i < m_ui->extensions->count(); ++i) {
    auto *chkbox = qobject_cast<ExtensionCheckBox *>(m_ui->extensions->itemAt(i)->widget());
    if (chkbox) {
      const bool extSupported = availableExtensions.contains( chkbox->extension() );
      chkbox->setEnabled(extSupported);
      
      if (!extSupported) {
        // shall not be checked at this point
        chkbox->setChecked(false);
        m_selectedExtensions->remove(*chkbox->extension());
      }
    }
  }

  //------------------------------------------------------------------------------
  // Option Management
  //------------------------------------------------------------------------------
  const auto availableOptions = variations.extractOptions();
  
  for (int i = 0; i < m_ui->options->count(); ++i) {
    auto *chkbox = qobject_cast<QCheckBox *>(m_ui->options->itemAt(i)->widget());
    if (chkbox) {
      const bool inOptions = availableOptions.contains(chkbox->text());
      chkbox->setEnabled(inOptions);
      
      if (!inOptions) {
        // shall not be checked at this point
        chkbox->setChecked(false);
        m_selectedOptions.erase(chkbox->text());
      }
    }
  }

  propagateVariationChange();
}

void ProcessorSelectionDialog::propagateVariationChange() {
  //------------------------------------------------------------------------------
  // After Filter Variation Selection
  //------------------------------------------------------------------------------
  const ProcClassInfo &desc = getSelectedProcessorClass();
  const bool validVariation = activeVariationIsValid();
  const auto varDesc = getSelectedVariation();

  //------------------------------------------------------------------------------
  // Layout Management
  //------------------------------------------------------------------------------
  
  if (validVariation) {
    auto active_layout = m_ui->layout->currentText();
    m_ui->layout->setEnabled(true);
    m_ui->layout->clear();

    for (const auto &layout : varDesc->layouts) {
      m_ui->layout->addItem(layout.name);

      if (layout.name == active_layout)
        m_ui->layout->setCurrentIndex(m_ui->layout->count() - 1);
    }
  } else {
    m_ui->layout->setEnabled(false);
  }

  //------------------------------------------------------------------------------
  // Statics Management
  //------------------------------------------------------------------------------
  m_ui->description->clear();

  if (validVariation) {
    // since the selected variation is valid the can set the infos accordingly
    m_ui->name->setText(varDesc->name);
    m_ui->isa->setText(varDesc->isaInfo().isa->CCmarch(*m_selectedExtensions));
    m_ui->description->appendHtml(varDesc->description);
  } else {
    // default setup based on the processor class info description
    m_ui->name->setText(desc.name);
    m_ui->isa->setText(ISAFamilyNames.at(desc.isa));
  }

  m_ui->name->setCursorPosition(0);
  m_ui->description->moveCursor(QTextCursor::Start);
  m_ui->description->ensureCursorVisible();

  m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validVariation);
  m_ui->regInitWidget->setEnabled(validVariation);
  m_ui->regInitWidget->processorSelectionChanged(m_selectedID, varDesc->variationInfo.id);
}


void ProcessorSelectionDialog::selectionChanged(QTreeWidgetItem *current,
                                                QTreeWidgetItem *) {
  if (current == nullptr) {
    return;
  }

  const bool validSelection = isCPUItem(current);
  m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validSelection);
  if (!validSelection) {
    // Something which is not a processor was selected (ie. an ISA). Disable OK
    // button
    return;
  }

  m_selectedID = qvariant_cast<ProcessorID>(current->data(ProcessorColumn, Qt::UserRole));
  
  const auto &desc = getSelectedProcessorClass();
  const auto varDesc = ProcessorRegistry::getDescription(m_selectedID, desc.defaultVariationID);

  m_selectedOptions = varDesc->variationInfo.options;
  m_selectedBitWidth = varDesc->variationInfo.bitWidth;
  m_selectedExtensions = varDesc->isaInfo().defaultExtensions->clone();

  buildSelector();
}

const Layout *ProcessorSelectionDialog::getSelectedLayout() const {
  if (!activeVariationIsValid())
    return nullptr;

  const auto desc = getSelectedVariation();

  auto it = llvm::find_if(desc->layouts, [&](const auto &layout) {
    return layout.name == m_ui->layout->currentText();
  });
  
  if (it != desc->layouts.end())
    return &*it;
  
  return nullptr;
}

} // namespace Ripes

#include "processorselectiondialog.moc"
