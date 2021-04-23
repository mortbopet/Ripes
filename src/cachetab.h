#pragma once

#include <QWidget>
#include "ripestab.h"

namespace Ripes {

namespace Ui {
class CacheTab;
}

class CacheTab : public RipesTab {
    Q_OBJECT

public:
    explicit CacheTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~CacheTab() override;

signals:
    void focusAddressChanged(uint32_t address);

private:
    Ui::CacheTab* m_ui;
};

}  // namespace Ripes
