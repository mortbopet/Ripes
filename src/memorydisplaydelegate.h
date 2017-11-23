#ifndef MEMORYDISPLAYDELEGATE_H
#define MEMORYDISPLAYDELEGATE_H

#include <QStyledItemDelegate>

#include "defines.h"

class MemoryDisplayDelegate : public QStyledItemDelegate {
public:
    MemoryDisplayDelegate(QWidget* parent = nullptr);

    QString displayText(const QVariant& value, const QLocale& locale) const override;

public slots:
    void setDisplayType(displayTypeN type);

private:
    displayTypeN m_displayType = displayTypeN::Hex;
};

#endif  // MEMORYDISPLAYDELEGATE_H
