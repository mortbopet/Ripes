#pragma once

#include <QDialog>
#include <QTreeWidget>

#include "processorregistry.h"

namespace Ripes {

namespace Ui {
class ProcessorSelectionDialog;
}

struct VariationSet {
  const ProcClassInfo& procDesc;
  std::set<const VariationInfo*> variations;

  using iterator = typename std::set<const VariationInfo*>::iterator;
  using const_iterator = typename std::set<const VariationInfo*>::const_iterator;

  iterator begin() { return variations.begin(); }
  iterator end() { return variations.end(); }
  const_iterator begin() const { return variations.begin(); }
  const_iterator end() const { return variations.end(); }
  iterator erase(iterator it) { return variations.erase(it); }

  size_t size() const { return variations.size(); }

  /// extract all options and extensions present in this variation set
  VariationInfo::Options_t extractOptions() const;
  ExtensionContainer_t extractExtensions() const;

  /// filter out variations that are not compatible with the given extensions
  VariationSet& filter(ExtensionSetInfo::ConstPtr extensions);

  /// filter out variations that are not compatible with the given options, 
  /// if matchExact is true, also filter out variations that contain options that are not selected
  VariationSet& filter(const VariationInfo::Options_t& options, bool matchExact);
};

class ProcessorSelectionDialog : public QDialog {
  Q_OBJECT

public:
  explicit ProcessorSelectionDialog(QWidget *parent = nullptr);
  ~ProcessorSelectionDialog();

  ProcessorID getSelectedId() const { return m_selectedID; }
  VariationID getSelectedVariationId() const {
    return getConfiguredVariationId().value_or(getSelectedProcessorClass().defaultVariationID);
  }
  ExtensionSetInfo::ConstPtr getEnabledExtensions() const { return m_selectedExtensions; }
  const ProcClassInfo& getSelectedProcessorClass() const {
    return ProcessorRegistry::getProcessorClassInfo(m_selectedID);
  }
  const std::shared_ptr<ProcVariationInfoBase> getSelectedVariation() const {
    return ProcessorRegistry::getDescription(m_selectedID, getSelectedVariationId());
  }

  const Layout *getSelectedLayout() const;
  RegisterInitialization getRegisterInitialization() const;

private slots:
  void selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
  // void optionChanged();
  // void extensionChanged();
  void variationSelectionChanged();

private:
  std::optional<VariationID> getConfiguredVariationId() const;

  bool activeVariationIsValid() const;
  
  /// get all available variations for the currently selected processor and bit-width
  VariationSet getAvailableVariations() const;
  /// get all variations matching the currently selected options and extensions
  VariationSet getMatchingVariations(bool matchExact) const;
  
  /// initialize the Selector widgets based on the currently selected processor and bit-width
  void buildSelector();
  
  void propagateVariationChange();

  bool isCPUItem(const QTreeWidgetItem *item) const;
  
  ProcessorID m_selectedID;
  ExtensionSetInfo::Ptr m_selectedExtensions;
  VariationInfo::Options_t m_selectedOptions;
  uint m_selectedBitWidth;
    
  enum ProcessorTreeColums { ProcessorColumn, ColumnCount };
  Ui::ProcessorSelectionDialog *m_ui;
};
} // namespace Ripes
