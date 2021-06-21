#include "rv32i_assembler.h"
#include "gnudirectives.h"
#include "ripessettings.h"
#include "rvrelocations.h"

#include <QByteArray>
#include <algorithm>

namespace Ripes {
namespace Assembler {

RV32I_Assembler::RV32I_Assembler(const ISAInfo<ISA::RV32I>* isa) : Assembler<Reg_T, Instr_T>(isa) {
    auto [instrs, pseudos] = initInstructions(isa);

    auto directives = gnuDirectives();
    auto relocations = rvRelocations<Reg_T, Instr_T>();
    initialize(instrs, pseudos, directives, relocations);

    // Initialize segment pointers
    setSegmentBase(".text", RipesSettings::value(RIPES_SETTING_ASSEMBLER_TEXTSTART).toUInt());
    setSegmentBase(".data", RipesSettings::value(RIPES_SETTING_ASSEMBLER_DATASTART).toUInt());
    setSegmentBase(".bss", RipesSettings::value(RIPES_SETTING_ASSEMBLER_BSSSTART).toUInt());

    // Monitor settings changes to segment pointers
    connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_TEXTSTART), &SettingObserver::modified,
            [this](const QVariant& value) { setSegmentBase(".text", value.toUInt()); });
    connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_DATASTART), &SettingObserver::modified,
            [this](const QVariant& value) { setSegmentBase(".data", value.toUInt()); });
    connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_BSSSTART), &SettingObserver::modified,
            [this](const QVariant& value) { setSegmentBase(".bss", value.toUInt()); });
}

std::tuple<RV32I_Assembler::_InstrVec, RV32I_Assembler::_PseudoInstrVec>
RV32I_Assembler::initInstructions(const ISAInfo<ISA::RV32I>* isa) const {
    _InstrVec instructions;
    _PseudoInstrVec pseudoInstructions;

    extI<Reg_T, Instr_T>::enable(isa, instructions, pseudoInstructions);
    for (const auto& extension : isa->enabledExtensions()) {
        switch (extension.unicode()->toLatin1()) {
            case 'M':
                extM<Reg_T, Instr_T>::enable(isa, instructions, pseudoInstructions);
                break;
            case 'F':
                enableExtF(isa, instructions, pseudoInstructions);
                break;
            default:
                assert(false && "Unhandled ISA extension");
        }
    }
    return {instructions, pseudoInstructions};
}

void RV32I_Assembler::enableExtF(const ISAInfoBase*, _InstrVec&, _PseudoInstrVec&) {
    // Pseudo-op functors

    // Assembler functors
}

}  // namespace Assembler
}  // namespace Ripes
