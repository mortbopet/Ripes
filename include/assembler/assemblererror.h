#pragma once

#include <QString>
#include <QTextStream>
#include <iostream>
#include <map>
#include <variant>
#include <vector>

#include "location.h"

namespace Ripes {
namespace Assembler {

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

} // namespace Assembler
} // namespace Ripes
