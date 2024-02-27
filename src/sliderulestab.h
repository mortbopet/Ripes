#pragma once

#include "isainfo.h"
#include "ripestab.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>
#include <QWidgetItem>

namespace Ripes {

namespace Ui {
class SliderulesTab;
}

class InstructionItem {
public:
  explicit InstructionItem(const InstructionBase &instruction,
                           InstructionItem *parent = nullptr);

  InstructionItem *child();
  // int childCount() const;
  const InstructionBase &data() const;
  InstructionItem *parent();
  bool hasParent() const;

private:
  std::vector<std::unique_ptr<InstructionItem>> m_children;
  const InstructionBase &m_instruction;
  InstructionItem *m_parent;
};

class ISAInstructionsModel : public QAbstractItemModel {
  Q_OBJECT
public:
  ISAInstructionsModel(QObject *parent = nullptr);
  ~ISAInstructionsModel() = default;

  virtual int rowCount(const QModelIndex &) const override;
  virtual int columnCount(const QModelIndex &) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override;
  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const override;
  virtual QModelIndex index(int row, int column,
                            const QModelIndex &parent = {}) const override;
  virtual QModelIndex parent(const QModelIndex &index) const override;

  const ISAInfoBase *isaInfo() const;
  const ISAInfoBase *prevISAInfo() const;

signals:
  void isaInfoChanged(const ISAInfoBase &isaInfo);

public slots:
  void changeISAFamily(ISAFamily isaFamily);
  void changeISA(ISA isa);
  void changeISAInfo(const ISAInfoBase &isaInfo);

protected:
  std::shared_ptr<const ISAInfoBase> m_isaInfo = nullptr;
  std::shared_ptr<const ISAInfoBase> m_prevIsaInfo = nullptr;

  std::vector<std::unique_ptr<InstructionItem>> m_instrItems;
};

class SliderulesTab : public RipesTab {
  Q_OBJECT
public:
  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

public slots:
  // void updateView(const ISAInfoBase &isaInfo);

private:
  void resetISAFilter(const ISAInfoBase &isaInfo);

  Ui::SliderulesTab *ui = nullptr;

  QComboBox *isaFamilyBox = nullptr;
  QComboBox *isaBox = nullptr;
  QComboBox *mainExtBox = nullptr;
  QCheckBox *baseExtCheckBox = nullptr;

  std::unique_ptr<ISAInstructionsModel> m_isaModel = nullptr;
};

} // namespace Ripes
