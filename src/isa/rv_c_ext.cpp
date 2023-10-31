#include "rv_c_ext.h"

namespace Ripes {
namespace RVISA {
namespace ExtC {

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &) {
  using namespace TypeCA;
  using namespace TypeCI;
  using namespace TypeCSS;
  using namespace TypeCL;
  using namespace TypeCS;
  using namespace TypeCJ;
  using namespace TypeCB;
  using namespace TypeCB2;
  using namespace TypeCIW;
  using namespace TypeCR;
  using namespace TypeCR2;

  enableInstructions<CSub, CXor, COr, CAnd, CLwsp, CFldsp, CSlli, CLi, CLui,
                     CAddi16Sp, CAddi, CNop, CSwsp, CFsdsp, CLw, CSw, CFsd, CJ,
                     CBeqz, CBnez, CSrli, CSrai, CAndi, CAddi4spn, CMv, CAdd,
                     CJr, CJalr>(instructions);

  if (isa->bits() == 32) {
    enableInstructions<CFlwsp, CFswsp, CFlw, CFsw, CJal>(instructions);
  } else {
    enableInstructions<CLdsp, CAddiw, CSdsp, CLd, CSd>(instructions);
  }

  if (isa->bits() == 64) {
    enableInstructions<CSubw, CAddw>(instructions);
  }
}

} // namespace ExtC
} // namespace RVISA
} // namespace Ripes
