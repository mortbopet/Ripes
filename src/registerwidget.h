#pragma once

#include <QWidget>

#include "processors/interface/ripesprocessor.h"

namespace Ripes {
class RegisterModel;

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget {
  Q_OBJECT

public:
  explicit RegisterWidget(const RegisterFileType regFileID,
                          QWidget *parent = nullptr);
  ~RegisterWidget();

  void initialize();

public slots:
  void updateView();
  void setRegisterviewCenterIndex(int index);

private:
  Ui::RegisterWidget *m_ui = nullptr;
  RegisterModel *m_model = nullptr;
  const RegisterFileType m_regFileID;
};
} // namespace Ripes
