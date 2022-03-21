#pragma once

#include <QString>
#include <cstdint>

namespace Ripes {
namespace Assembler {

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
} // namespace Assembler
} // namespace Ripes
