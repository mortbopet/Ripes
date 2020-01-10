#ifndef GOTOCOMBOBOX_H
#define GOTOCOMBOBOX_H

#include <QComboBox>

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
};

#endif  // GOTOCOMBOBOX_H
