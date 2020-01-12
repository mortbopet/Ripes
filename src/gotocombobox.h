#ifndef GOTOCOMBOBOX_H
#define GOTOCOMBOBOX_H

#include <QComboBox>
#include <QMetaType>

enum class GoToFunction { Select, Address };
Q_DECLARE_METATYPE(GoToFunction);

class GoToComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit GoToComboBox(QWidget* parent = nullptr);
    void showPopup() override;

signals:
    void indexChanged();
    void jumpToAddress(uint32_t address);

public slots:

private:
    void signalFilter(int index);
};

#endif  // GOTOCOMBOBOX_H
