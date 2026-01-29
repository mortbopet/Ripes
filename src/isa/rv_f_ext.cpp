#include "rv_f_ext.h"

namespace Ripes {
namespace RVISA {
namespace ExtF {

// clang-format off
static void _enableExtPseudo(const ISAInfoBase *,
                             PseudoInstrVec &pseudoInstructions) {
  using namespace TypePseudo;

  enablePseudoInstructions< Flw, Fsw, 
                            Fmv::s, Fabs::s, Fneg::s,
                            FmvAlias::s::x, FmvAlias::x::s,
                            TypeCSR::Frcsr, TypeCSR::Fscsr,
                            TypeCSR::Frrm, TypeCSR::Fsrm,
                            TypeCSR::Frflags, TypeCSR::Fsflags
                          >(pseudoInstructions);
}

// Enable 64-bit extensions
static void _enableExt64(const ISAInfoBase *, InstrVec &instructions) {
  using namespace TypeR;

  enableInstructions< Fcvt::l::s, Fcvt::lu::s, Fcvt::s::l, Fcvt::s::lu>(instructions);
}

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions) {
  _enableExtPseudo(isa, pseudoInstructions);

  using namespace TypeI;
  using namespace TypeS;
  using namespace TypeR;
  using namespace TypeR4;

  enableInstructions< Flw, Fsw,
                      Fmadd::s, Fmsub::s, Fnmsub::s, Fnmadd::s,
                      Fadd::s, Fsub::s, Fmul::s, Fdiv::s, Fsqrt::s,
                      Fsgnj::s, Fsgnjn::s, Fsgnjx::s,
                      Fmin::s, Fmax::s,
                      Fcvt::w::s, Fcvt::wu::s, Fcvt::s::w, Fcvt::s::wu,
                      Fmv::w::x, Fmv::x::w,
                      Feq::s, Flt::s, Fle::s,
                      Fclass::s >(instructions);

  if (isa->bits() == 64) {
    _enableExt64(isa, instructions);
  }
}
// clang-format on

} // namespace ExtI
} // namespace RVISA
} // namespace Ripes
