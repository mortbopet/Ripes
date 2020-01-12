#ifndef MEMORYTAB_H
#define MEMORYTAB_H

#include "defines.h"

#include <QWidget>

#include <unordered_map>

#include "memorymodel.h"
#include "processorhandler.h"
#include "ripestab.h"

namespace Ui {
class MemoryTab;
}

class MemoryTab : public RipesTab {
    Q_OBJECT

public:
    MemoryTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~MemoryTab() override;

public slots:
    void update();

private:
    Ui::MemoryTab* m_ui;
};

#endif  // MEMORYTAB_H
