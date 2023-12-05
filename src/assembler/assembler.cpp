#include "assembler.h"

namespace Ripes {
namespace Assembler {

std::shared_ptr<AssemblerBase>
constructAssemblerDynamic(const std::shared_ptr<const ISAInfoBase> &isa) {
  if (auto rv32isa =
          std::dynamic_pointer_cast<const ISAInfo<ISA::RV32I>>(isa)) {
    return std::make_shared<ISA_Assembler<ISA::RV32I>>(rv32isa);
  } else if (auto rv64isa =
                 std::dynamic_pointer_cast<const ISAInfo<ISA::RV64I>>(isa)) {
    return std::make_shared<ISA_Assembler<ISA::RV64I>>(rv64isa);
  }

  Q_UNREACHABLE();
}

} // namespace Assembler
} // namespace Ripes
