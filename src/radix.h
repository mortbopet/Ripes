#pragma once

#include <QMetaType>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QString>

#include <climits>
#include <map>

#include "ripes_types.h"

namespace Ripes {

enum class Radix { Hex, Binary, Unsigned, Signed, ASCII, Float };
const static std::map<Radix, QString> s_radixName = {
    {Radix::Hex, "Hex"},           {Radix::Binary, "Binary"},
    {Radix::Unsigned, "Unsigned"}, {Radix::Signed, "Signed"},
    {Radix::ASCII, "ASCII"},       {Radix::Float, "Float"}};

static const auto hexRegex = QRegularExpression("0[xX][0-9a-fA-F]+");
static const auto hexRegex16 = QRegularExpression("0[xX][0-9a-fA-F]{0,4}");
static const auto hexRegex32 = QRegularExpression("0[xX][0-9a-fA-F]{0,8}");
static const auto hexRegex64 = QRegularExpression("0[xX][0-9a-fA-F]{0,16}");
static const auto binRegex = QRegularExpression("0[bB][0-1]+");
static const auto unsignedRegex = QRegularExpression("[0-9]+");
static const auto signedRegex = QRegularExpression("[-]*[0-9]+");

void setISADepRegex(QRegularExpressionValidator *validator);
QString encodeRadixValue(VInt value, const Radix type, unsigned byteWidth);
VInt decodeRadixValue(QString value, const Radix type, bool *ok = nullptr);

} // namespace Ripes

Q_DECLARE_METATYPE(Ripes::Radix);
