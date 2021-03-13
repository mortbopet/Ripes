#pragma once

#include <QSpinBox>

namespace Ripes {

class HexSpinBox : public QSpinBox {
public:
    HexSpinBox(QWidget* parent = 0) : QSpinBox(parent) {
        setPrefix("0x");
        setDisplayIntegerBase(16);
        setRange(INT_MIN, INT_MAX);
    }

protected:
    QString textFromValue(int value) const override {
        return QString::number(static_cast<unsigned>(value), 16).rightJustified(8, '0').toUpper();
    }
    int valueFromText(const QString& text) const override {
        int v = sanitize(text).toUInt(nullptr, 16);
        return v;
    }
    QValidator::State validate(QString& input, int& pos) const {
        bool okay;
        unsigned int val = sanitize(input).toUInt(&okay, 16);
        if (!okay)
            return QValidator::Invalid;
        return QValidator::Acceptable;
    }

private:
    QString sanitize(const QString& input) const {
        QString copy(input);
        if (copy.startsWith("0x"))
            copy.remove(0, 2);
        return copy;
    }
};

}  // namespace Ripes
