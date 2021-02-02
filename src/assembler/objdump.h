#pragma once

#include "binutils.h"
#include "defines.h"
#include "program.h"

#include <QString>

#include <memory>

namespace Ripes {
namespace Assembler {

/**
 * AddrOffsetMap
 * To preserve accurate mappings between the program viewers disassembled view - which contains additional lines for
 * marking symbols, and the addresses of the current program, we provide the AddrOffsetMap.
 * Keys represent the editor line number of which an additional line was inserted, in addition to lines corresponding to
 * the disassembled program.
 * The value is a pair containing:
 *  first: # of offset lines up to (not including) the given offset
 *  second: An optional QString() referencing the symbol displayed at the offset.
 */
using AddrOffsetMap = std::map<unsigned long, std::pair<int, QString>>;

QString objdump(std::shared_ptr<const Program> program, AddrOffsetMap& addrOffsetMap);
QString binobjdump(std::shared_ptr<const Program> program, AddrOffsetMap& addrOffsetMap);

}  // namespace Assembler
}  // namespace Ripes
