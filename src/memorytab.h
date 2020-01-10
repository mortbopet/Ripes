#ifndef MEMORYTAB_H
#define MEMORYTAB_H

#include "defines.h"

#include <QWidget>

#include <unordered_map>

#include "mainmemory.h"
#include "memorydisplaydelegate.h"
#include "memorymodel.h"
#include "processorhandler.h"
#include "ripestab.h"

namespace Ui {
class MemoryTab;
}

class RegisterWidget;

class MemoryTab : public RipesTab {
    Q_OBJECT

public:
    MemoryTab(ProcessorHandler& handler, QToolBar* toolbar, QWidget* parent = nullptr);
    ~MemoryTab() override;
    void initMemoryTab();

public slots:
    void saveAddress();

    void update();
    void jumpToAdress(uint32_t address);

private:
    void initializeMemoryView();
    void initializeRegisterView();

    Ui::MemoryTab* m_ui;
    MemoryModel* m_model;
    MemoryDisplayDelegate* m_delegate;

    MainMemory* m_memoryPtr;
    std::vector<uint32_t>* m_regPtr;

    std::vector<RegisterWidget*> m_regWidgetPtrs;

    ProcessorHandler& m_handler;
};

#endif  // MEMORYTAB_H
