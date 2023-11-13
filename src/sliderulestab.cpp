#include "sliderulestab.h"
#include "ui_sliderulestab.h"

namespace Ripes {

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), ui(new Ui::SliderulesTab) {
  ui->setupUi(this);
}

SliderulesTab::~SliderulesTab() { delete ui; }

} // namespace Ripes
