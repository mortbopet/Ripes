#pragma once

#include "ripestab.h"
#include <QWidget>

#include "ripes_types.h"

namespace Ripes {

namespace Ui {
class BranchTab;
}

class BranchTab : public RipesTab {
  Q_OBJECT

public:
  explicit BranchTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~BranchTab() override;

  void tabVisibilityChanged(bool visible) override;

  void updateStatistics();

  void setupTables(int rows, int colums);

  void updateTables();

signals:
  void focusAddressChanged(Ripes::AInt address);

private:
  Ui::BranchTab *m_ui;
  bool m_initialized = false;
  QTimer *m_statUpdateTimer;
  int table_rows = 32;
  int table_columns = 4;
  int num_history_bits = 8;
  int num_prediction_bits = 2;
};

} // namespace Ripes
