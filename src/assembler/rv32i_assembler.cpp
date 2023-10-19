#include "rv32i_assembler.h"
#include "gnudirectives.h"
#include "ripessettings.h"
#include "rvrelocations.h"

#include <QByteArray>
#include <algorithm>

namespace Ripes {
namespace Assembler {

RV32I_Assembler::RV32I_Assembler(const ISAInfo<ISA::RV32I> *isa)
    : Assembler(isa) {
  auto [instrs, pseudos] = initInstructions(isa);

  auto directives = gnuDirectives();
  auto relocations = rvRelocations();
  initialize(instrs, pseudos, directives, relocations);

  // Initialize segment pointers and monitor settings changes to segment
  // pointers
  connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_TEXTSTART),
          &SettingObserver::modified, this, [this](const QVariant &value) {
            setSegmentBase(".text", value.toULongLong());
          });
  RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_TEXTSTART)->trigger();
  connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_DATASTART),
          &SettingObserver::modified, this, [this](const QVariant &value) {
            setSegmentBase(".data", value.toULongLong());
          });
  RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_DATASTART)->trigger();
  connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_BSSSTART),
          &SettingObserver::modified, this, [this](const QVariant &value) {
            setSegmentBase(".bss", value.toULongLong());
          });
  RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_BSSSTART)->trigger();
}

std::tuple<InstrVec, PseudoInstrVec>
RV32I_Assembler::initInstructions(const ISAInfo<ISA::RV32I> *isa) const {
  InstrVec instructions;
  PseudoInstrVec pseudoInstructions;

  //  RV_I<Reg_T>::enable(isa, instructions, pseudoInstructions);
  //  for (const auto &extension : isa->enabledExtensions()) {
  //    switch (extension.unicode()->toLatin1()) {
  //    case 'M':
  //      RV_M<Reg_T>::enable(isa, instructions, pseudoInstructions);
  //      break;
  //    case 'C':
  //      RV_C<Reg_T>::enable(isa, instructions, pseudoInstructions);
  //      break;
  //    default:
  //      assert(false && "Unhandled ISA extension");
  //    }
  //  }
  return {instructions, pseudoInstructions};
}

} // namespace Assembler
} // namespace Ripes
