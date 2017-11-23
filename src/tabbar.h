#ifndef TABBAR_H
#define TABBAR_H

#include <QLabel>
#include <QListWidget>

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

#endif  // TABBAR_H
