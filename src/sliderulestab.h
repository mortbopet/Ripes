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

class ISAFamilyComboBox : public QComboBox {
  Q_OBJECT
public:
  ISAFamilyComboBox(QWidget *parent = nullptr);

  ISAFamily currentFamily() const { return currentData().value<ISAFamily>(); }

public slots:
  /// Updates the UI and emits `familyChanged()` signal.
  void setFamily(ISAFamily family);

signals:
  /// Called when the family is changed in the UI directly by the user.
  void familyActivated(ISAFamily family);
  /// Called when any time the family is changed, including during
  /// `setFamily()`.
  void familyChanged(ISAFamily family);
};

class ISAComboBox : public QComboBox {
  Q_OBJECT
public:
  ISAComboBox(QWidget *parent = nullptr);

  ISA currentISA() const { return currentData().value<ISA>(); }

public slots:
  /// Updates the UI and emits `isaChanged()` signal.
  void setISA(ISA isa);
  /// Updates the UI using the new family and emits `isaChanged()` signal.
  void setFamily(ISAFamily family);

signals:
  /// Called when the isa is changed in the UI directly by the user.
  void isaActivated(ISA isa);
  /// Called when any time the isa is changed, including during
  /// `setFamily()`.
  void isaChanged(ISA isa);
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
