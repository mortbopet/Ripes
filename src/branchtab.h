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

  void updateRuntimeFacts();

  void setupTables(int rows1, int colums1, int rows2, int columns2);

  void updateTables();

  void procChanged();

  void predictorChanged(bool is_preset);

signals:
  void focusAddressChanged(Ripes::AInt address);

private:
  Ui::BranchTab *m_ui;
  bool m_initialized = false;
  QTimer *m_statUpdateTimer;
  int table1_rows = 0;
  int table1_columns = 0;
  int table2_rows = 0;
  int table2_columns = 0;
  uint16_t num_address_bits = 0;
  uint16_t num_history_bits = 0;
  uint16_t num_state_bits = 0;
  bool is_branch_proc = true;
  uint8_t predictor = 0;
};

} // namespace Ripes
