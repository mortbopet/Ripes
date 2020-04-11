#pragma once

#include <QWidget>

namespace Ripes {

namespace Ui {
class CacheWidget;
}

class CacheWidget : public QWidget {
    Q_OBJECT

public:
    explicit CacheWidget(QWidget* parent = nullptr);
    ~CacheWidget();

private:
    Ui::CacheWidget* m_ui;
};

}  // namespace Ripes
