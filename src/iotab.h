#pragma once

#include "defines.h"

#include <QAction>
#include <QWidget>

#include <unordered_map>

#include "memorymodel.h"
#include "processorhandler.h"
#include "ripestab.h"

namespace Ripes {

namespace Ui {
class IOTab;
}

class IOTab : public RipesTab {
    Q_OBJECT

public:
    IOTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~IOTab() override;

private:
    void tile();

    Ui::IOTab* m_ui = nullptr;

    QAction* m_tileAction = nullptr;
};
}  // namespace Ripes
