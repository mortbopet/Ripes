#pragma once
#include <QWidget>

namespace Ripes {
class MemoryTab;
class CacheWidget;

namespace Ui {
class CacheTabWidget;
}

class CacheTabWidget : public QWidget {
    Q_OBJECT

public:
    explicit CacheTabWidget(QWidget* parent = nullptr);
    ~CacheTabWidget();

signals:
    void focusAddressChanged(unsigned address);

private:
    void connectCacheWidget(CacheWidget* w);

    Ui::CacheTabWidget* m_ui;
    MemoryTab* m_parent = nullptr;
};

}  // namespace Ripes
