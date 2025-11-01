#include "assembler.h"
#include "assembler/special_assembler/assemblerVLIW.h"
#include "isa/rvVliwIsainfo.h"

namespace Ripes {
namespace Assembler {

template <ISA _isa>
static std::shared_ptr<AssemblerBase> genericAssemblerBase(std::shared_ptr<ISAInfoBase> isa) {
  // detect VLIW isa
  if (auto baseVLIWIsa = std::dynamic_pointer_cast<ISAVliwInfo<_isa>>(isa)) {
    return std::make_shared<ISA_Assembler<_isa, AssemblerVLIW>>(baseVLIWIsa);
  }
  
  // else try matching basic ISA
  if (auto baseIsa = std::dynamic_pointer_cast<ISAInfo<_isa>>(isa)) {
    return std::make_shared<ISA_Assembler<_isa>>(baseIsa);
  }

  return nullptr;
}

std::shared_ptr<AssemblerBase>
constructAssemblerDynamic(std::shared_ptr<ISAInfoBase> isa) {
  // try getting an assembler for an 32-bit isa
  if (auto rv32isa = genericAssemblerBase<ISA::RV32I>(isa)) {
    return rv32isa;
  }

  // else try getting an assembler for an 64-bit isa
  if (auto rv64isa = genericAssemblerBase<ISA::RV64I>(isa)) {
    return rv64isa;
  }

  throw std::runtime_error(
      std::string("Cannot dynamically construct assembler for isa: ") +
      isa->name().toStdString());
  return {};
}

} // namespace Assembler
} // namespace Ripes
