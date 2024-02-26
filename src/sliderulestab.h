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

class ISAInstructionsModel : public QStandardItemModel {
  Q_OBJECT
public:
  ISAInstructionsModel(QObject *parent = nullptr);
  ~ISAInstructionsModel() = default;

  virtual int rowCount(const QModelIndex &) const override;
  virtual int columnCount(const QModelIndex &) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const override;

  //   const ISAInfoBase *isaInfo() const;
  //   const ISAInfoBase *prevISAInfo() const;

  // signals:
  //   void isaInfoChanged(const ISAInfoBase &isaInfo);

  // public slots:
  //   void changeISAFamily(ISAFamily isaFamily);
  //   void changeISA(ISA isa);
  //   void changeISAInfo(const ISAInfoBase &isaInfo);

protected:
  // std::shared_ptr<const ISAInfoBase> m_isaInfo = nullptr;
  // std::shared_ptr<const ISAInfoBase> m_prevIsaInfo = nullptr;
};

class SliderulesTab : public RipesTab {
  Q_OBJECT
public:
  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

public slots:
  // void updateView(const ISAInfoBase &isaInfo);

private:
  Ui::SliderulesTab *ui = nullptr;

  QComboBox *isaFamilyBox = nullptr;
  QComboBox *isaBox = nullptr;
  QComboBox *mainExtBox = nullptr;
  QCheckBox *baseExtCheckBox = nullptr;

  std::unique_ptr<ISAInstructionsModel> m_isaModel = nullptr;
};

} // namespace Ripes
