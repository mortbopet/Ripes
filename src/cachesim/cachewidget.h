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

    void setNextLevelCache(std::shared_ptr<CacheSim>& cache);

    std::shared_ptr<CacheSim>& getCacheSim() { return m_cacheSim; }

signals:
    void cacheAddressSelected(uint32_t);
    void configurationChanged();

private:
    Ui::CacheWidget* m_ui;
    std::shared_ptr<CacheSim> m_cacheSim;
};

}  // namespace Ripes
