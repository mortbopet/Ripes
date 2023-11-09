#include "rv_m_ext.h"
namespace Ripes {
namespace RVISA {
namespace ExtM {

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &) {
  using namespace TypeR;

  enableInstructions<Mul, Mulh, Mulhsu, Mulhu, Div, Divu, Rem, Remu>(
      instructions);

  if (isa->bits() == 64) {
    enableInstructions<Mulw, Divw, Divuw, Remw, Remuw>(instructions);
  }
}

} // namespace ExtM
} // namespace RVISA
} // namespace Ripes
