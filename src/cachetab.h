#pragma once

#include "ripestab.h"
#include <QWidget>

#include "isa/isa_types.h"

namespace Ripes {

namespace Ui {
class CacheTab;
}

class CacheTab : public RipesTab {
  Q_OBJECT

public:
  explicit CacheTab(QToolBar *toolbar, QWidget *parent = nullptr);
  ~CacheTab() override;

  void tabVisibilityChanged(bool visible) override;

signals:
  void focusAddressChanged(Ripes::AInt address);

private:
  Ui::CacheTab *m_ui;
  bool m_initialized = false;
};

} // namespace Ripes
