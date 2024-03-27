#pragma once

#include "ripestab.h"
#include "edittab.h"
#include "taskchecker.h"

namespace Ripes {

namespace Ui {
class TaskTab;
}

class TaskTab : public RipesTab {
  Q_OBJECT

public slots:
  void changeSection();
  void changeTask();
  void checkTask();

public:
  TaskTab(QToolBar *toolbar, EditTab *edittab, QWidget *parent = nullptr);
  ~TaskTab() override;

private:
  unsigned int currentNumber;
  unsigned int currentSection;
  TaskChecker taskchecker;
  EditTab* edittab;
  Ui::TaskTab *m_ui = nullptr;  
};
} // namespace Ripes
