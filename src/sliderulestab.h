#pragma once

#include "isainfo.h"
#include "ripestab.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QWidget>
#include <QWidgetItem>

namespace Ripes {

namespace Ui {
class SliderulesTab;
}

class ISAEncodingTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  ISAEncodingTableModel(QObject *parent = nullptr);

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
  //  virtual QVariant headerData(int section, Qt::Orientation orientation,
  //                              int role = Qt::DisplayRole) const override;

public slots:
  void change(const QString &s);

protected:
  std::shared_ptr<const ISAInfoBase> m_isaInfo = nullptr;
  std::shared_ptr<const ISAInfoBase> m_prevIsaInfo = nullptr;

  QStringList tmp;
};

class SliderulesTab : public RipesTab {
  Q_OBJECT
public:
  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

public slots:
  void initializeView(const ISAInfoBase &isaInfo);
  void updateView(const ISAInfoBase &isaInfo, const ISAInfoBase &prevISAInfo);

private:
  Ui::SliderulesTab *ui = nullptr;

  QComboBox *isaFamilyBox = nullptr;
  QComboBox *isaBox = nullptr;
  QComboBox *mainExtBox = nullptr;

  std::unique_ptr<ISAEncodingTableModel> m_encodingModel = nullptr;
};

} // namespace Ripes
