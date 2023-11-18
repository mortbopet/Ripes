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
  explicit RegisterWidget(const std::string_view &regFileID,
                          QWidget *parent = nullptr);
  ~RegisterWidget();

  void initialize();

public slots:
  void updateView();
  void setRegisterviewCenterIndex(int index);

private:
  void showContextMenu(const QPoint &pos);
  Ui::RegisterWidget *m_ui = nullptr;
  RegisterModel *m_model = nullptr;
  const std::string_view m_regFileName;
};
} // namespace Ripes
