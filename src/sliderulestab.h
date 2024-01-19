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

class ISAFamilySelector : public QComboBox {
  Q_OBJECT
public:
  ISAFamilySelector(QWidget *parent = nullptr);

  ISAFamily currentFamily() const;

public slots:
  /// Updates the UI and emits `familyChanged()` signal.
  void setFamily(ISAFamily family);

signals:
  /// Called when the family is changed in the UI directly by the user.
  void familyActivated(ISAFamily family);
  /// Called when any time the ISA family is changed, including during
  /// `setFamily()`.
  void familyChanged(ISAFamily family);
};

class ISAEncodingTableView : public QTableView {
  Q_OBJECT
public:
  ISAEncodingTableView(QWidget *parent = nullptr);

public slots:
  /// Updates the UI and emits `familyChanged()` signal.
  void setFamily(ISAFamily family);
  //  void setISA(ISA isa);
  //  void setExtension(QString ext, bool enabled);

signals:
  void familyChanged(ISAFamily family);
};

class SliderulesTab : public RipesTab {
  Q_OBJECT
public:
  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

private:
  Ui::SliderulesTab *ui;
  std::unique_ptr<ISAEncodingTableModel> m_encodingModel = nullptr;
};

} // namespace Ripes
