#include "rv_zicsr_ext.h"

namespace Ripes {
namespace RVISA {
namespace ExtZicsr {

static void _enableExtPseudo(const ISAInfoBase *,
                             PseudoInstrVec &pseudoInstructions) {
  using namespace TypePseudo;

  enablePseudoInstructions<Csrr, Csrs, Csrc, Csrw, Csrsi, Csrci, Csrwi>(pseudoInstructions);
}

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions) {
  _enableExtPseudo(isa, pseudoInstructions);

  using namespace TypeCSR;

  enableInstructions<Csrrw, Csrrs, Csrrc, Csrrwi, Csrrsi, Csrrci>(instructions);
}

} // namespace ExtZicsr
} // namespace RVISA
} // namespace Ripes
