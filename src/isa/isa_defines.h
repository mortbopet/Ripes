#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <set>

#include <QList>
#include <QString>
#include <QTextStream>

#include "ripes_types.h"

namespace Ripes {

using Instr_T = uint64_t;
using Reg_T = uint64_t;

enum class Radix { Hex, Binary, Unsigned, Signed, ASCII, Float };

struct ImmConvInfo {
  bool isUnsigned = false;
  bool is32bit = false;
  Radix radix;
};

inline int64_t getImmediate(const QString &string, bool &canConvert,
                            ImmConvInfo *convInfo = nullptr) {
  QString upperString = string.toUpper();
  QString trimmed;
  canConvert = false;
  int64_t immediate = upperString.toLongLong(&canConvert, 10);
  int64_t sign = 1;
  if (!canConvert) {
    // Could not convert directly to integer - try hex or bin. Here, extra
    // care is taken to account for a potential sign, and include this is the
    // range validation
    if (upperString.size() > 0 &&
        (upperString.at(0) == '-' || upperString.at(0) == '+')) {
      sign = upperString.at(0) == '-' ? -1 : 1;
      upperString.remove(0, 1);
    }
    if (upperString.startsWith(QLatin1String("0X"))) {
      trimmed = upperString.remove("0X");
      if (convInfo) {
        convInfo->isUnsigned = true;
        convInfo->is32bit = trimmed.size() <= 8;
        convInfo->radix = Radix::Hex;
      }
      immediate = trimmed.toULongLong(&canConvert, 16);
    } else if (upperString.startsWith(QLatin1String("0B"))) {
      trimmed = upperString.remove("0B");
      if (convInfo) {
        convInfo->isUnsigned = true;
        convInfo->is32bit = trimmed.size() <= 32;
        convInfo->radix = Radix::Binary;
      }
      immediate = trimmed.toULongLong(&canConvert, 2);
    } else {
      canConvert = false;
    }
  } else {
    if (convInfo) {
      convInfo->radix = Radix::Signed;
    }
  }

  return sign * immediate;
}

inline int64_t getImmediateSext32(const QString &string, bool &success,
                                  ImmConvInfo *convInfo = nullptr) {
  std::unique_ptr<ImmConvInfo> innerConvInfo;
  if (!convInfo) {
    innerConvInfo = std::make_unique<ImmConvInfo>();
    convInfo = innerConvInfo.get();
  }

  int64_t value = getImmediate(string, success, convInfo);

  // This seems a tad too specific for RISC-V, but the official RISC-V tests
  // expects the immediate of i.e., "andi x14, x1, 0xffffff0f" to be accepted
  // as a signed immediate, even in 64-bit.
  if (success && (static_cast<uint32_t>(value >> 32) == 0) &&
      convInfo->is32bit) {
    value = static_cast<int32_t>(value);
  }
  return value;
}

class Token : public QString {
public:
  Token(const QString &t) : QString(t) {}
  Token(const QString &t, const QString &relocation)
      : QString(t), m_relocation(relocation) {}
  Token() : QString() {}
  void setRelocation(const QString &relocation) { m_relocation = relocation; }
  bool hasRelocation() const { return !m_relocation.isEmpty(); }
  const QString &relocation() const { return m_relocation; }

private:
  QString m_relocation;
};

using LineTokens = QVector<Token>;
using LineTokensVec = std::vector<LineTokens>;

struct Symbol {
public:
  enum Type { Address = 1 << 0, Constant = 1 << 1 };
  Symbol(){};
  Symbol(const char *str) : v(str) {}
  Symbol(const QString &str) : v(str) {}
  Symbol(const QString &str, const Type _type) : v(str), type(_type) {}

  bool operator==(const Symbol &rhs) const { return this->v == rhs.v; }
  bool operator<(const Symbol &rhs) const { return this->v < rhs.v; }

  bool is(const Type t) const { return type & t; }
  bool is(const unsigned t) const { return type & t; }

  // A local symbol is a numerical label.
  bool isLocal() const {
    bool ok;
    (void)v.toUInt(&ok);
    return ok;
  }

  /// Returns true if this symbol is legal. An illegal symbal is any symbol that
  /// starts with a numeric value which is not a number.
  bool isLegal() const {
    bool startsWithNum;
    QString(v.front()).toInt(&startsWithNum);
    if (startsWithNum && !isLocal())
      return false;
    return true;
  }

  operator const QString &() const { return v; }

  QString v;
  unsigned type = 0;
};

using Symbols = std::set<Symbol>;
using DirectiveLinePair = std::pair<QString, LineTokens>;

class Location {
public:
  int64_t sourceLine() const { return m_sourceLine; };
  Location(int64_t sourceLine) : m_sourceLine(sourceLine) {
    assert(sourceLine >= 0 &&
           "Use Location::unknown() to construct unknown locations");
  }
  QString toString() const {
    return isKnownSourceLine() ? "UNKNOWN" : QString::number(m_sourceLine);
  }
  bool operator==(const Location &other) const {
    return this->sourceLine() == other.sourceLine();
  }
  bool operator!=(const Location &other) const { return !(*this == other); }
  bool isKnownSourceLine() const { return *this != Location::unknown(); }
  static Location unknown() {
    auto loc = Location(0);
    loc.m_sourceLine = -1;
    return loc;
  }

private:
  int64_t m_sourceLine;
};

struct TokenizedSrcLine : public Location {
  explicit TokenizedSrcLine(unsigned sourceLine) : Location(sourceLine) {}
  Symbols symbols;
  LineTokens tokens;
  QString directive;
  AInt programAddress = -1;
};

/// An error is defined as a reference to a source line index + an error string
class Error : public Location {
public:
  Error(const Location &location, const QString &message)
      : Location(location), m_message(message) {}
  const QString &errorMessage() const { return m_message; }
  friend QTextStream &operator<<(QTextStream &stream, const Error &err);

private:
  // Error message to report
  QString m_message;
};

inline QTextStream &operator<<(QTextStream &stream, const Error &err) {
  stream << QString("line ") << err.toString() << " : " << err.errorMessage();
  return stream;
}

class Errors : public std::vector<Error> {
public:
  void print() const {
    std::cout << toString().toStdString();
    std::cout << std::flush;
  }

  QString toString() const {
    QString str;
    auto strStream = QTextStream(&str);
    for (const auto &iter : *this) {
      strStream << iter << "\n";
    }
    return str;
  }

  /**
   * @brief toMap
   * @return map representation of [source line: error message].
   * A map cache is created whenever a discrepancy between size(this) !=
   * size(_mapcache). @todo this obviously assumes that any element pushed onto
   * *this error vector is never modified or removed from the set of errors. A
   * bit iffy, but currently, Errors objects are returned from the Assemblers
   * and intended to be immutable outside of the assembler.
   */
  const std::map<unsigned, QString> &toMap() const {
    if (_mapcache.size() != size()) {
      _mapcache.clear();
      std::map<unsigned, QString> m;
      for (const auto &iter : *this) {
        _mapcache[iter.sourceLine()] = iter.errorMessage();
      }
    }
    return _mapcache;
  }

private:
  mutable std::map<unsigned, QString> _mapcache;
};

/// An Assembler Result structure is a variant which carries either an error
/// message or some result value.
/// Monostate used in cases of no return result, but still error-able.
template <typename T = std::monostate>
struct Result : public std::variant<Error, T> {
  using V_T = std::variant<Error, T>;

  Result(const T &v) : V_T(v) {}

  Result(const Error &err) : V_T(err) {}

  const T &value() {
    auto opt = std::get_if<T>(this);
    if (opt) {
      return *opt;
    }
    assert(false && "Optional<T>::value() called on an Optional<Error>");
    return std::get<T>(*this); // Make compiler happy
  }

  const Error &error() {
    auto opt = std::get_if<Error>(this);
    if (opt) {
      return *opt;
    }
    assert(false && "Optional<T>::error() called on an Optional<T>");
    return std::get<Error>(*this); // Make compiler happy
  }

  bool isError() const { return std::holds_alternative<Error>(*this); }
  bool isResult() const { return std::holds_alternative<T>(*this); }

  // Convenience function for a default constructed value of T.
  static T def() { return std::monostate(); }
};

} // namespace Ripes
