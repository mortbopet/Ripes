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

// Enable 64-bit extensions
static void _enableExt64(const ISAInfoBase *, InstrVec &instructions,
                         const std::set<Options> &) {
  using namespace TypeL;
  using namespace TypeR;
  using namespace TypeS;
  using namespace TypeIShift64;

  enableInstructions<Addw, Subw, Sllw, Srlw, Sraw, Slli, Srli, Srai, Lwu, Ld,
                     Sd>(instructions);
}

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions,
               const std::set<Options> &options) {
  _enableExtPseudo(isa, pseudoInstructions, options);

  using namespace TypeI;
  using namespace TypeIShift;
  using namespace TypeL;
  using namespace TypeSystem;
  using namespace TypeU;
  using namespace TypeJ;
  using namespace TypeS;
  using namespace TypeR;
  using namespace TypeB;

  enableInstructions<Addi, Andi, Slti, Sltiu, Xori, Ori, Lb, Lh, Lw, Lbu, Lhu,
                     Ecall, Auipc, Lui, Jal, Jalr, Sb, Sw, Sh, Add, Sub, Sll,
                     Slt, Sltu, Xor, Srl, Sra, Or, And, Beq, Bne, Blt, Bge,
                     Bltu, Bgeu>(instructions);

  if (options.count(Options::shifts64BitVariant)) {
    // 64-bit shift instructions
    enableInstructions<Slliw, Srliw, Sraiw>(instructions);
  } else {
    // 32-bit shift instructions
    enableInstructions<Slli, Srli, Srai>(instructions);
  }

  if (isa->bits() == 64) {
    _enableExt64(isa, instructions, options);
  }
}

} // namespace ExtI
} // namespace RVISA
} // namespace Ripes
