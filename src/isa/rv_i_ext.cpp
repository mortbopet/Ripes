#include "rv_i_ext.h"

namespace Ripes {
namespace RVISA {
namespace ExtI {

using namespace TypeI;

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions) {
  enablePseudoInstructions<Lb, Lh, Lw>(pseudoInstructions);
  enableInstructions<AddI, AndI>(instructions);
}

} // namespace ExtI
} // namespace RVISA
} // namespace Ripes
