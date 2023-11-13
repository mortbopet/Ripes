#pragma once

#include "instruction.h"
#include "isainfo.h"
#include "processorhandler.h"
#include "pseudoinstruction.h"
#include "ripestab.h"

#include <QHeaderView>
#include <QWidget>

namespace Ripes {

namespace Ui {
class SliderulesTab;
}

class SliderulesModel : public QAbstractTableModel {
  Q_OBJECT
public:
  SliderulesModel(
      const ISAInfoBase *isa,
      const std::shared_ptr<const InstrVec> instructions,
      const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
      QObject *parent = nullptr);
  ~SliderulesModel();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override = 0;

protected:
  const ISAInfoBase *m_isa;
  const std::shared_ptr<const InstrVec> m_instructions;
  const std::shared_ptr<const PseudoInstrVec> m_pseudoInstructions;
};

class SliderulesEncodingModel final : public SliderulesModel {
  Q_OBJECT
public:
  SliderulesEncodingModel(
      const ISAInfoBase *isa,
      const std::shared_ptr<const InstrVec> instructions,
      const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
      QObject *parent = nullptr);

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
};

class SliderulesDecodingModel final : public SliderulesModel {
  Q_OBJECT
public:
  SliderulesDecodingModel(
      const ISAInfoBase *isa,
      const std::shared_ptr<const InstrVec> instructions,
      const std::shared_ptr<const PseudoInstrVec> pseudoInstructions,
      QObject *parent = nullptr);

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
};

class SliderulesTab : public RipesTab {
  Q_OBJECT

public:
  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

  void setData(const ISAInfoBase *isa, InstrVec instructions,
               PseudoInstrVec pseudoInstructions);
  void updateTables();

public slots:
  void onProcessorChanged();

private:
  Ui::SliderulesTab *ui;
  const ISAInfoBase *m_isa;
  std::shared_ptr<const InstrVec> m_instructions;
  std::shared_ptr<const PseudoInstrVec> m_pseudoInstructions;
  std::unique_ptr<SliderulesDecodingModel> m_decodingModel;
  std::unique_ptr<SliderulesEncodingModel> m_encodingModel;
};

} // namespace Ripes
