#include "rv_c_ext.h"

namespace Ripes {
namespace RVISA {
namespace ExtC {

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &) {
  using namespace TypeCA;

  enableInstructions<CSub, CXor, COr, CAnd>(instructions);

  if (isa->bits() == 64) {
    enableInstructions<CSubw, CAddw>(instructions);
  }
}

} // namespace ExtC
} // namespace RVISA
} // namespace Ripes
