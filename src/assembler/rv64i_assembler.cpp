#include "rv64i_assembler.h"
#include "gnudirectives.h"
#include "ripessettings.h"
#include "rvrelocations.h"

#include <QByteArray>
#include <algorithm>

#include "isa/rv_c_ext.h"
#include "isa/rv_i_ext.h"
#include "isa/rv_m_ext.h"

namespace Ripes {
namespace Assembler {

RV64I_Assembler::RV64I_Assembler(const ISAInfo<ISA::RV64I> *isa)
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
RV64I_Assembler::initInstructions(const ISAInfo<ISA::RV64I> *isa) const {
  InstrVec instructions;
  PseudoInstrVec pseudoInstructions;

  RVISA::ExtI::enableExt(isa, instructions, pseudoInstructions,
                         {RVISA::ExtI::Options::shifts64BitVariant,
                          RVISA::ExtI::Options::LI64BitVariant});
  for (const auto &extension : isa->enabledExtensions()) {
    switch (extension.unicode()->toLatin1()) {
    case 'M':
      RVISA::ExtM::enableExt(isa, instructions, pseudoInstructions);
      break;
    case 'C':
      RVISA::ExtC::enableExt(isa, instructions, pseudoInstructions);
      break;
    }
  }

  return {instructions, pseudoInstructions};
}

} // namespace Assembler
} // namespace Ripes
