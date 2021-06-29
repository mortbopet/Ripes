#pragma once

#include <QWidget>

#include <unordered_map>

#include "memorymodel.h"
#include "ripestab.h"

namespace Ripes {

namespace Ui {
class MemoryTab;
}

class MemoryTab : public RipesTab {
    friend class CacheTabWidget;
    Q_OBJECT

public:
    MemoryTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~MemoryTab() override;

    void setCentralAddress(unsigned address);

private:
    Ui::MemoryTab* m_ui = nullptr;
};
}  // namespace Ripes
