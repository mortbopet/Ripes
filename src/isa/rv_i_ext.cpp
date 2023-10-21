#include "rv_i_ext.h"

namespace Ripes {
namespace RVISA {
namespace ExtI {

using namespace TypeI;
using namespace TypeIShift;

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions) {
  enablePseudoInstructions<Lb, Lh, Lw>(pseudoInstructions);
  enableInstructions<Addi, Andi, Slti, Sltiu, Xori, Ori, Slli, Srli, Srai>(
      instructions);
}

} // namespace ExtI
} // namespace RVISA
} // namespace Ripes
