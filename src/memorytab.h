#pragma once

#include "defines.h"

#include <QWidget>

#include <unordered_map>

#include "memorymodel.h"
#include "processorhandler.h"
#include "ripestab.h"

namespace Ripes {

namespace Ui {
class MemoryTab;
}

class MemoryTab : public RipesTab {
    Q_OBJECT

public:
    MemoryTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~MemoryTab() override;

signals:
    void reqProcessorReset();

public slots:
    void update();

private:
    Ui::MemoryTab* m_ui = nullptr;
};
}  // namespace Ripes
