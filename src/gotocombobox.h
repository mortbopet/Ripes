#ifndef GOTOCOMBOBOX_H
#define GOTOCOMBOBOX_H

#include <QComboBox>
#include <QItemDelegate>

class ComboboxDelegate : public QItemDelegate {
    Q_OBJECT
public:
    explicit ComboboxDelegate(QWidget* parent = nullptr);

    void mousePressEvent(QMouseEvent* event);
};

class GoToComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit GoToComboBox(QWidget* parent = nullptr);

signals:
    void indexChanged();
    void jumpToAddress(uint32_t address);

public slots:

private:
    void signalFilter(int index);

    // ComboboxDelegate m_delegate;
};

#endif  // GOTOCOMBOBOX_H
