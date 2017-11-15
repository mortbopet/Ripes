#include "tabbar.h"

#include <QLabel>
#include <QVBoxLayout>

TabItem::TabItem(QString iconPath, QString name, TabBar* parent) : QWidget(parent) {
    m_name = new QLabel(name);

    auto iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(iconPath));
    auto layout = new QVBoxLayout();

    layout->addWidget(iconLabel);
    layout->addWidget(m_name);

    setLayout(layout);
}

// ---------------- TabBar ---------------------

TabBar::TabBar(QWidget* parent) : QListWidget(parent) {}

void TabBar::addTabItem(QString label, QString iconPath) {
    auto listItem = new QListWidgetItem(label);
    auto tabItem = new TabItem(iconPath, label, this);

    setItemWidget(listItem, tabItem);

    addItem(listItem);
}
