#include "rv_i_ext.h"

namespace Ripes {
namespace RVISA {
namespace ExtI {

static void _enableExtPseudo(const ISAInfoBase *,
                             PseudoInstrVec &pseudoInstructions,
                             const std::set<Options> &) {
  using namespace TypePseudo;

  enablePseudoInstructions<Lb, Lh, Lw>(pseudoInstructions);
}

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions,
               const std::set<Options> &options) {
  _enableExtPseudo(isa, pseudoInstructions, options);

  using namespace TypeI;
  using namespace TypeIShift;
  using namespace TypeL;
  using namespace TypeSystem;

  enableInstructions<Addi, Andi, Slti, Sltiu, Xori, Ori, Lb, Lh, Lw, Lbu, Lhu,
                     Ecall>(instructions);

  if (options.count(Options::shifts64BitVariant)) {
    // 64-bit shift instructions
    enableInstructions<Slliw, Srliw, Sraiw>(instructions);
  } else {
    // 32-bit shift instructions
    enableInstructions<Slli, Srli, Srai>(instructions);
  }
}

} // namespace ExtI
} // namespace RVISA
} // namespace Ripes
