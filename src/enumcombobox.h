#pragma once

#include <QComboBox>
#include <QString>
#include <map>

namespace Ripes {

template <typename Enum>
void setupEnumCombobox(QComboBox* combobox, const std::map<Enum, QString>& nameMap) {
    for (const auto& iter : nameMap) {
        combobox->addItem(iter.second, QVariant::fromValue(iter.first));
    }
}

template <typename Enum>
void setEnumIndex(QComboBox* combobox, Enum enumItem) {
    for (int i = 0; i < combobox->count(); i++) {
        if (qvariant_cast<Enum>(combobox->itemData(i)) == enumItem) {
            combobox->setCurrentIndex(i);
            return;
        }
    }
    Q_ASSERT(false && "Index not found");
}

template <typename Enum>
Enum getEnumValue(QComboBox* combobox) {
    return qvariant_cast<Enum>(combobox->itemData(combobox->currentIndex()));
}

}  // namespace Ripes
