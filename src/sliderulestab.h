#pragma once

#include "ripestab.h"

namespace Ripes {

namespace Ui {
class SliderulesTab;
}

class SliderulesTab : public RipesTab {
  Q_OBJECT

public:
  SliderulesTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~SliderulesTab() override;

private:
  Ui::SliderulesTab *m_ui = nullptr;
};

}
