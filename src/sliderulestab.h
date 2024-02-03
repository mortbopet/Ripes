#pragma once

#include "isainfo.h"
#include "ripestab.h"
#include "sliderulesmodels.h"

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

class ISAEncodingFilters : public QFrame {
  Q_OBJECT
public:
  ISAEncodingFilters(QWidget *parent = nullptr);

  QComboBox *isaFamilyBox;
  QComboBox *isaBox;
  QComboBox *mainExtBox;

signals:
  void isaFamilyChanged(ISAFamily isaFamily);

public slots:
  void initializeView(const ISAInfoBase &isaInfo);
  void updateView(const ISAInfoBase &isaInfo, const ISAInfoBase &prevISAInfo);
};

class ISAEncodingTableView : public QTableView {
  Q_OBJECT
public:
  ISAEncodingTableView(QWidget *parent = nullptr);

public slots:
  void initializeView(const ISAInfoBase &isaInfo);
  void updateView(const ISAInfoBase &isaInfo, const ISAInfoBase &prevISAInfo);
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
  Ui::SliderulesTab *ui;
  std::unique_ptr<ISAEncodingTableModel> m_encodingModel = nullptr;
};

} // namespace Ripes
