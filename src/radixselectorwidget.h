#ifndef RADIXSELECTORWIDGET_H
#define RADIXSELECTORWIDGET_H

#include <QWidget>

#include "radix.h"

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
    void radixChanged(Radix r);

private:
    Ui::RadixSelectorWidget* m_ui;
};

#endif  // RADIXSELECTORWIDGET_H
