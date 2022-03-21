#include "symbolmap.h"

namespace Ripes {
namespace Assembler {

/// Adds a symbol to the current symbol mapping of this assembler.
std::optional<Error> SymbolMap::addAbsSymbol(const unsigned &line,
                                             const Symbol &s, VInt v) {
  if (abs.count(s)) {
    return {Error(line, "Multiple definitions of symbol '" + s.v + "'")};
  }
  abs[s] = v;
  return {};
}

std::optional<Error> SymbolMap::addRelSymbol(const unsigned &line,
                                             const Symbol &s, VInt v) {
  assert(s.isLocal());
  auto &it = rel[s.v.toInt()];
  if (it.count(line))
    return {Error(line, QString::fromStdString(
                            "Multiple definitions of relative symbol '" +
                            std::to_string(v) + "' on line '" +
                            std::to_string(line)))};
  it[line] = v;
  return {};
}

AbsoluteSymbolMap SymbolMap::copyRelativeTo(unsigned line,
                                            const QString &beforeSuffix,
                                            const QString &afterSuffix) const {
  AbsoluteSymbolMap res = abs;

  for (auto &relSymbols : rel) {
    auto ub = relSymbols.second.upper_bound(line);
    if (ub != relSymbols.second.end())
      res[QString::number(relSymbols.first) + afterSuffix] = ub->second;

    if (ub != relSymbols.second.begin()) {
      auto lb = ub;
      std::advance(lb, -1);
      res[QString::number(relSymbols.first) + beforeSuffix] = lb->second;
    }
  }

  return res;
}

} // namespace Assembler
} // namespace Ripes
