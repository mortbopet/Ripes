#include "rv64i_assembler.h"
#include "gnudirectives.h"
#include "ripessettings.h"
#include "rv32i_assembler.h"
#include "rvassembler_common.h"
#include "rvrelocations.h"

#include <QByteArray>
#include <algorithm>

namespace Ripes {
namespace Assembler {

RV64I_Assembler::RV64I_Assembler(const ISAInfo<ISA::RV64I>* isa) : Assembler(isa) {
    auto [instrs, pseudos] = initInstructions(isa);

    auto directives = gnuDirectives();
    auto relocations = rvRelocations();
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

std::tuple<InstrVec, PseudoInstrVec> RV64I_Assembler::initInstructions(const ISAInfo<ISA::RV64I>* isa) const {
    InstrVec instructions;
    PseudoInstrVec pseudoInstructions;

    enableExtI(isa, instructions, pseudoInstructions);
    for (const auto& extension : isa->enabledExtensions()) {
        switch (extension.unicode()->toLatin1()) {
            case 'M':
                enableExtM(isa, instructions, pseudoInstructions);
                break;
            default:
                assert(false && "Unhandled ISA extension");
        }
    }
    return {instructions, pseudoInstructions};
}

void RV64I_Assembler::enableExtI(const ISAInfoBase* isa, InstrVec& instructions, PseudoInstrVec& pseudoInstructions) {
    RV32I_Assembler::enableExtI(isa, instructions, pseudoInstructions, {RV32I_Assembler::Options::shifts64BitVariant});

    instructions.push_back(IType32(Token("addiw"), 0b000));

    instructions.push_back(IShiftType64(Token("slli"), RVISA::OPIMM, 0b001, 0b000000));
    instructions.push_back(IShiftType64(Token("srli"), RVISA::OPIMM, 0b101, 0b000000));
    instructions.push_back(IShiftType64(Token("srai"), RVISA::OPIMM, 0b101, 0b010000));

    instructions.push_back(RType32(Token("addw"), 0b000, 0b0000000));
    instructions.push_back(RType32(Token("subw"), 0b000, 0b0100000));
    instructions.push_back(RType32(Token("sllw"), 0b001, 0b0000000));
    instructions.push_back(RType32(Token("srlw"), 0b101, 0b0000000));
    instructions.push_back(RType32(Token("sraw"), 0b101, 0b0100000));
}

void RV64I_Assembler::enableExtM(const ISAInfoBase* isa, InstrVec& instructions, PseudoInstrVec& pseudoInstructions) {
    RV32I_Assembler::enableExtM(isa, instructions, pseudoInstructions);
}

}  // namespace Assembler
}  // namespace Ripes
