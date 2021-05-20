#pragma once

#include <QWidget>

#include "radix.h"

namespace Ripes {

namespace Ui {
class RadixSelectorWidget;
}

class RadixSelectorWidget : public QWidget {
    Q_OBJECT

public:
    explicit RadixSelectorWidget(QWidget* parent = nullptr);
    ~RadixSelectorWidget();
    void setupRadixComboBox();
    void setRadix(Radix r);

signals:
    void radixChanged(Ripes::Radix r);

private:
    Ui::RadixSelectorWidget* m_ui = nullptr;
};
}  // namespace Ripes
