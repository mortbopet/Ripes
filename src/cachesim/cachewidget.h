#pragma once

#include <QWidget>
#include "cachesim.h"
#include "processors/RISC-V/rv_memory.h"

using RVMemory = vsrtl::core::RVMemory<32, 32>;

namespace Ripes {

class CacheSim;

namespace Ui {
class CacheWidget;
}

class CacheWidget : public QWidget {
    Q_OBJECT

public:
    explicit CacheWidget(QWidget* parent = nullptr);
    ~CacheWidget();

    void setType(CacheSim::CacheType type);

signals:
    void cacheAddressSelected(uint32_t);
    void configurationChanged();

private:
    Ui::CacheWidget* m_ui;
    CacheSim* m_cacheSim = nullptr;
};

}  // namespace Ripes
