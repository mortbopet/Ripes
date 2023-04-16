#pragma once

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QWidget>

#include "processorregistry.h"

namespace Ripes {
class RegisterInitializationWidget;
class RegisterSelectionComboBox;

namespace Ui {
class RegisterInitializationWidget;
}

class RegisterInitializationWidget : public QWidget {
  Q_OBJECT
  friend class RegisterSelectionComboBox;

  struct RegInitWidgets {
    RegisterSelectionComboBox *name = nullptr;
    QLineEdit *value = nullptr;
    QPushButton *remove = nullptr;

    ~RegInitWidgets() { clear(); }
    void clear();
  };

public:
  explicit RegisterInitializationWidget(QWidget *parent = nullptr);
  ~RegisterInitializationWidget();

  RegisterInitialization getInitialization() const;

  void processorSelectionChanged(ProcessorID id);

private slots:
  RegisterInitializationWidget::RegInitWidgets *
  addRegisterInitialization(unsigned regIdx);

private:
  void updateAddButtonState();
  int getNonInitializedRegIdx();
  void removeRegInitWidget(RegInitWidgets *w);

  Ui::RegisterInitializationWidget *m_ui = nullptr;

  static std::map<ProcessorID, RegisterInitialization> m_initializations;
  ProcessorID m_currentID;
  QRegularExpressionValidator *m_hexValidator;

  std::vector<std::unique_ptr<RegInitWidgets>> m_currentRegInitWidgets;
};

class RegisterSelectionComboBox : public QComboBox {
  Q_OBJECT
public:
  RegisterSelectionComboBox(RegisterInitializationWidget *parent = nullptr);
  void showPopup() override;

signals:
  void regIndexChanged(int oldIdx, int newIdx);

private:
  int m_index = -1;
  RegisterInitializationWidget *m_parent = nullptr;
};

} // namespace Ripes
