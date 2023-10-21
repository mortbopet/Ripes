#include "rv_i_ext.h"

namespace Ripes {
namespace RVISA {
namespace ExtI {

using namespace TypeI;
using namespace TypeIShift;

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions,
               const std::set<Options> &options) {
  enablePseudoInstructions<Lb, Lh, Lw>(pseudoInstructions);
  enableInstructions<Addi, Andi, Slti, Sltiu, Xori, Ori>(instructions);

  if (options.count(Options::shifts64BitVariant)) {
    // 64-bit shift instructions
  } else {
    // 32-bit shift instructions
    enableInstructions<Slli, Srli, Srai>(instructions);
  }
}

} // namespace ExtI
} // namespace RVISA
} // namespace Ripes
