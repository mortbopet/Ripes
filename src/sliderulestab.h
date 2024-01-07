#pragma once

#include "instruction.h"
#include "isainfo.h"
#include "pseudoinstruction.h"
#include "qcombobox.h"
#include "qlayoutitem.h"
#include "ripestab.h"

#include <QHeaderView>
#include <QTableView>
#include <QWidget>

namespace Ripes {

namespace Ui {
class SliderulesTab;
}

class EncodingView;

class EncodingModel : public QAbstractTableModel {
  Q_OBJECT
public:
  EncodingModel(const std::shared_ptr<const ISAInfoBase> isa,
                const std::shared_ptr<const InstrVec> instructions,
                const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
                QObject *parent = nullptr);

  // TODO(raccog): This could be a variable that is controlable in the UI.
  constexpr static size_t BIT_COLUMNS = 32;

  enum Column {
    EXTENSION = 0,
    TYPE,
    DESCRIPTION,
    OPCODE,
    // TODO(raccog): The field columns should be variable based on the maximum
    // number of fields
    FIELD0,
    FIELD1,
    FIELD2,
    BIT_START,
    BIT_END = BIT_START + BIT_COLUMNS
  };

  virtual int rowCount(const QModelIndex &) const override;
  virtual int columnCount(const QModelIndex &) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const override;

  const std::shared_ptr<const ISAInfoBase> isa;
  const std::shared_ptr<const InstrVec> instructions;
  const std::shared_ptr<const PseudoInstrVec> pseudoInstructions;

protected:
  friend class EncodingView;

  /// A set of all the row numbers that represent immediate encodings.
  std::set<int> m_immediateRows;
};

class EncodingView : public QTableView {
  Q_OBJECT
public:
  EncodingView(QComboBox &isaFamilySelector, QComboBox &regWidthSelector,
               QComboBox &mainExtensionSelector, QWidget *parent = nullptr);

  // TODO(raccog): Make this protected.
  std::unique_ptr<EncodingModel> m_model;

public slots:
  void processorChanged();

private slots:
  void isaFamilyChanged(int index);
  void regWidthChanged(int index);
  void mainExtensionChanged(int index);

protected:
  void updateModel(std::shared_ptr<const ISAInfoBase> isa);
  void updateView();

  QComboBox &m_isaFamilySelector;
  QComboBox &m_regWidthSelector;
  QComboBox &m_mainExtensionSelector;
};

struct DecodingModel : public QAbstractTableModel {
  DecodingModel(const std::shared_ptr<const ISAInfoBase> isa,
                const std::shared_ptr<const InstrVec> instructions,
                QObject *parent = nullptr);

  virtual int rowCount(const QModelIndex &) const override;
  virtual int columnCount(const QModelIndex &) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override;

protected:
  const std::shared_ptr<const ISAInfoBase> m_isa;
  const std::shared_ptr<const InstrVec> m_instructions;
};

class DecodingView : public QTableView {
  Q_OBJECT
public:
  DecodingView(QWidget *parent = nullptr);

protected:
  std::unique_ptr<DecodingModel> m_model;
};

class SliderulesTab : public RipesTab {
  Q_OBJECT
public:
  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

private:
  Ui::SliderulesTab *ui;

  // These pointers are owned by the Qt layout and should not be deleted in the
  // destructor.
  EncodingView *m_encodingTable;
};

} // namespace Ripes
