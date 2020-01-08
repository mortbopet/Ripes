#pragma once

#include <QMetaType>
#include <QRegExp>
#include <QString>
#include <map>

enum class Radix { Hex, Binary, Unsigned, Signed };
const static std::map<Radix, QString> s_radixName = {{Radix::Hex, "Hex"},
                                                     {Radix::Binary, "Binary"},
                                                     {Radix::Unsigned, "Unsigned"},
                                                     {Radix::Signed, "Signed"}};

Q_DECLARE_METATYPE(Radix);

static const auto hexRegex = QRegExp("0[xX][0-9a-fA-F]+");
static const auto binRegex = QRegExp("0[bB][0-1]+");
static const auto unsignedRegex = QRegExp("[0-9]+");
static const auto signedRegex = QRegExp("[-]*[0-9]+");

static QString encodeRadixValue(uint32_t value, const Radix type) {
    switch (type) {
        case Radix::Hex: {
            return "0x" + QString::number(value, 16).rightJustified(32 / 4, '0');
        }
        case Radix::Binary: {
            return "0b" + QString::number(value, 2).rightJustified(32, '0');
        }
        case Radix::Unsigned: {
            return QString::number(value, 10);
        }
        case Radix::Signed: {
            return QString::number(static_cast<int32_t>(value), 10);
        }
        default:
            return QString();
    }
}
