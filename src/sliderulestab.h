#pragma once

#include "ripestab.h"

#include <QWidget>

namespace Ripes {

namespace Ui {
class SliderulesTab;
}

class SliderulesTab : public RipesTab {
  Q_OBJECT

public:
  explicit SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab();

private:
  Ui::SliderulesTab *ui;
};

} // namespace Ripes
