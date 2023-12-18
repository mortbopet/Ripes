#include "isaconstructor.h"

#include "mips32isainfo.h"
#include "rv32isainfo.h"
#include "rv64isainfo.h"

namespace Ripes {

template <ISA... isas>
std::map<ISA,
         std::function<std::shared_ptr<const ISAInfoBase>(const QStringList &)>>
_constructConstructors() {
  std::map<ISA, std::function<std::shared_ptr<const ISAInfoBase>(
                    const QStringList &)>>
      constructors;
  (
      [&] {
        constructors[isas] = [](const QStringList &extensions) {
          return std::make_shared<ISAInfo<isas>>(extensions);
        };
      }(),
      ...);
  return constructors;
}

std::map<ISA,
         std::function<std::shared_ptr<const ISAInfoBase>(const QStringList &)>>
constructConstructors() {
  /// NOTE: Ensure that every ISA is used in the `_constructConstructors<...>()`
  /// template
  return _constructConstructors<ISA::MIPS32I, ISA::RV32I, ISA::RV64I>();
}

} // namespace Ripes
