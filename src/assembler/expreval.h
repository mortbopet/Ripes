#pragma once

#include <variant>
#include "assemblererror.h"

namespace Ripes {
namespace AssemblerTmp {

/**
 * @brief evaluate
 * Very simple expression parser for evaluating a right-associative binary (2-operand) mathematical expressions.
 * For now, no operator precedence is implemented - to ensure precedence, parentheses must be implemented. The
 * functionality is mainly intended to be used by the assembler to expand complex pseudoinstructions and as such not by
 * the user.
 */
std::variant<Error, long> evaluate(const QString&, const std::map<QString, uint32_t>* variables = nullptr);

}  // namespace AssemblerTmp
}  // namespace Ripes
