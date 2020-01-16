#pragma once

#include <QComboBox>
#include <QMetaType>

namespace Ripes {

enum class GoToFunction { Select, Address, Section };

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
}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::GoToFunction);
