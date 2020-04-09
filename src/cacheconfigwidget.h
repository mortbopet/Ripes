#pragma once

#include <QWidget>
#include "cachebase.h"

namespace Ripes {

namespace Ui {
class CacheConfigWidget;
}

class CacheConfigWidget : public QWidget {
    Q_OBJECT
public:
    CacheConfigWidget(QWidget* parent);
    ~CacheConfigWidget() override;

    void setCache(CacheBase* cache);

private:
    CacheBase* m_cache;
    Ui::CacheConfigWidget* m_ui = nullptr;
};

}  // namespace Ripes
