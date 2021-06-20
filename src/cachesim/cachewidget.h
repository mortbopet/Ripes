#pragma once

#include <QWidget>
#include "cachesim.h"
#include "processors/RISC-V/rv_memory.h"

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)

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

    void setNextLevelCache(const std::shared_ptr<CacheSim>& cache);

    std::shared_ptr<CacheSim>& getCacheSim() { return m_cacheSim; }
    QGraphicsScene* getScene() { return m_scene.get(); }

private:
    Ui::CacheWidget* m_ui;
    std::shared_ptr<CacheSim> m_cacheSim;
    std::unique_ptr<QGraphicsScene> m_scene;
};

}  // namespace Ripes
