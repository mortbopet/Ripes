#include "radix.h"

#include "processorhandler.h"

namespace Ripes {

void setISADepRegex(QRegularExpressionValidator *validator) {
  const auto isaBytes = ProcessorHandler::currentISA()->bytes();
  if (isaBytes == 2) {
    validator->setRegularExpression(hexRegex16);
  } else if (isaBytes == 4) {
    validator->setRegularExpression(hexRegex32);
  } else if (isaBytes == 8) {
    validator->setRegularExpression(hexRegex64);
  } else {
    Q_UNREACHABLE();
  }
}

QString encodeRadixValue(VInt value, const Radix type, unsigned byteWidth) {
  switch (type) {
  case Radix::Hex: {
    return "0x" + QString::number(value, 16).rightJustified(byteWidth * 2, '0');
  }
  case Radix::Float: {
    float _fvalue; // Copy raw data instead of reinterpret_cast to avoid
                   // type-punned pointer error
    double _dvalue;
    if (byteWidth <= 4) {
      memcpy(&_fvalue, &value, sizeof(_fvalue));
      return QString::number(_fvalue);
    } else {
      memcpy(&_dvalue, &value, sizeof(_dvalue));
      return QString::number(_dvalue);
    }
  }
  case Radix::Binary: {
    return "0b" +
           QString::number(value, 2).rightJustified(byteWidth * CHAR_BIT, '0');
  }
  case Radix::Unsigned: {
    return QString::number(value, 10);
  }
  case Radix::Signed: {
    if (byteWidth == 4) {
      return QString::number(static_cast<int32_t>(value), 10);
    } else {
      return QString::number(static_cast<int64_t>(value), 10);
    }
  }
  case Radix::ASCII: {
    QString str;
    for (unsigned i = 0; i < byteWidth; ++i) {
      str.prepend(QChar::fromLatin1(value & 0xFF));
      value >>= 8;
    }
    return str;
  }
  }
  Q_UNREACHABLE();
}

VInt decodeRadixValue(QString value, const Radix type, bool *ok) {
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
    for (const auto &c : value) {
      valueRev.prepend(c);
    }
    uint32_t v = 0;
    for (int i = 0; i < valueRev.length(); ++i) {
      v |= (valueRev[i].toLatin1() & 0xFF) << (i * 8);
    }
    *ok = true;
    return v;
  }
  }
  Q_UNREACHABLE();
}

} // namespace Ripes
