#pragma once

#include <QWidget>

namespace Ripes {

namespace Ui {
class ConsoleWidget;
}

class ConsoleWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConsoleWidget(QWidget* parent = nullptr);
    ~ConsoleWidget();

    void putData(const QByteArray& data);
    void clearConsole();

private:
    Ui::ConsoleWidget* m_ui;
};

}  // namespace Ripes
