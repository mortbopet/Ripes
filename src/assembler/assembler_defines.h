#pragma once

#include <QByteArray>
#include <QStringList>

#include <map>
#include <optional>
#include <set>
#include <variant>

#include "isa/isa_defines.h"
#include "program.h"

namespace Ripes {
namespace Assembler {

using SymbolLinePair = std::pair<Symbols, LineTokens>;
using SourceProgram = std::vector<TokenizedSrcLine>;
using NoPassResult = std::monostate;
using Section = QString;

/**
 * @brief The Result struct
 * An assembly result is determined to be valid iff errors is empty.
 */
struct AssembleResult {
  Errors errors;
  Program program;
};

struct DisassembleResult {
  Errors errors;
  QStringList program;
};

} // namespace Assembler

} // namespace Ripes
