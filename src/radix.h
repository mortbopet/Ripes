#pragma once

#include <QMetaType>
#include <QRegExp>
#include <QString>
#include <map>

namespace Ripes {

enum class Radix { Hex, Binary, Unsigned, Signed, ASCII, Float };
const static std::map<Radix, QString> s_radixName = {{Radix::Hex, "Hex"},           {Radix::Binary, "Binary"},
                                                     {Radix::Unsigned, "Unsigned"}, {Radix::Signed, "Signed"},
                                                     {Radix::ASCII, "ASCII"},       {Radix::Float, "Float"}};

static const auto hexRegex = QRegExp("0[xX][0-9a-fA-F]+");
static const auto hexRegex32 = QRegExp("0[xX][0-9a-fA-F]{0,8}");
static const auto binRegex = QRegExp("0[bB][0-1]+");
static const auto unsignedRegex = QRegExp("[0-9]+");
static const auto signedRegex = QRegExp("[-]*[0-9]+");

static QString encodeRadixValue(unsigned long value, const Radix type, unsigned width = 32) {
    switch (type) {
        case Radix::Hex: {
            return "0x" + QString::number(value, 16).rightJustified(width / 4, '0');
        }
        case Radix::Float: {
            return QString::number(*reinterpret_cast<float*>(&value));
        }
        case Radix::Binary: {
            return "0b" + QString::number(value, 2).rightJustified(width, '0');
        }
        case Radix::Unsigned: {
            return QString::number(value, 10);
        }
        case Radix::Signed: {
            return QString::number(static_cast<int32_t>(value), 10);
        }
        case Radix::ASCII: {
            QString str;
            for (unsigned i = 0; i < width / 8; i++) {
                str.prepend(QChar::fromLatin1(value & 0xFF));
                value >>= 8;
            }
            return str;
        }
    }
    Q_UNREACHABLE();
}

static uint32_t decodeRadixValue(QString value, const Radix type, bool* ok = nullptr) {
    switch (type) {
        case Radix::Hex: {
            return value.toUInt(ok, 16);
        }
        case Radix::Binary: {
            // Qt doesn't support 0b[0-1]* conversion, so remove any possible 0b prefix
            if (value.startsWith("0b")) {
                value.remove(0, 2);
            }
            return value.toUInt(ok, 2);
        }
        case Radix::Unsigned: {
            return value.toUInt(ok, 10);
        }
        case Radix::Signed: {
            return value.toInt(ok, 10);
        }
        case Radix::Float: {
            return value.toFloat(ok);
        }
        case Radix::ASCII: {
            QString valueRev;
            for (const auto& c : value) {
                valueRev.prepend(c);
            }
            uint32_t v = 0;
            for (int i = 0; i < valueRev.length(); i++) {
                v |= (valueRev[i].toLatin1() & 0xFF) << (i * 8);
            }
            *ok = true;
            return v;
        }
    }
    Q_UNREACHABLE();
}

}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::Radix);
