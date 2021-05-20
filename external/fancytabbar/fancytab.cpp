#include "fancytab.h"

#include <QDebug>

FancyTab::FancyTab(const QIcon& icon, QString text) {
    m_icon = icon;
    m_text = text;
}
