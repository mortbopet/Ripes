#pragma once

#include <QLabel>
#include <QListWidget>

namespace Ripes {

class TabBar;

class TabItem : public QWidget {
    Q_OBJECT
public:
    TabItem(QString iconPath, QString name, TabBar* parent = nullptr);

private:
    QIcon* m_icon;
    QLabel* m_name;
};

class TabBar : public QListWidget {
    Q_OBJECT
public:
    explicit TabBar(QWidget* parent = nullptr);

    void addTabItem(QString label, QString iconPath);
};

}  // namespace Ripes
