#pragma once

#include <QWidget>

namespace Ripes {

namespace Ui {
class RegisterContainerWidget;
}

class RegisterContainerWidget : public QWidget {
  Q_OBJECT

public:
  explicit RegisterContainerWidget(QWidget *parent = nullptr);
  ~RegisterContainerWidget();

private:
  void initialize();
  void updateView();
  Ui::RegisterContainerWidget *m_ui;
};

} // namespace Ripes
