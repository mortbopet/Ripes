#include "rv64i_assembler.h"
#include "gnudirectives.h"
#include "ripessettings.h"
#include "rv32i_assembler.h"
#include "rvassembler_common.h"
#include "rvrelocations.h"

#include <QByteArray>
#include <algorithm>

#include "rv_c_ext.h"
#include "rv_i_ext.h"
#include "rv_m_ext.h"

namespace Ripes {
namespace Assembler {

RV64I_Assembler::RV64I_Assembler(const ISAInfo<ISA::RV64I> *isa)
    : Assembler<Reg_T>(isa) {
  auto [instrs, pseudos] = initInstructions(isa);

  auto directives = gnuDirectives();
  auto relocations = rvRelocations<Reg_T>();
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

std::tuple<RV64I_Assembler::_InstrVec, RV64I_Assembler::_PseudoInstrVec>
RV64I_Assembler::initInstructions(const ISAInfo<ISA::RV64I> *isa) const {
  _InstrVec instructions;
  _PseudoInstrVec pseudoInstructions;

  enableExtI(isa, instructions, pseudoInstructions);
  for (const auto &extension : isa->enabledExtensions()) {
    switch (extension.unicode()->toLatin1()) {
    case 'M':
      enableExtM(isa, instructions, pseudoInstructions);
      break;
    case 'C':
      RV_C<Reg_T>::enable(isa, instructions, pseudoInstructions);
      break;
    default:
      assert(false && "Unhandled ISA extension");
    }
  }
  return {instructions, pseudoInstructions};
}

void RV64I_Assembler::enableExtI(const ISAInfoBase *isa,
                                 _InstrVec &instructions,
                                 _PseudoInstrVec &pseudoInstructions) {
  RV_I<Reg_T>::enable(isa, instructions, pseudoInstructions,
                      {RV_I<Reg_T>::Options::shifts64BitVariant,
                       RV_I<Reg_T>::Options::LI64BitVariant});

  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrI32Type<Reg_T>(Token("addiw"), 0b000, isa)));

  instructions.push_back(
      std::shared_ptr<_Instruction>(new RVInstrIShift64Type<Reg_T>(
          Token("slli"), RVISA::OPIMM, 0b001, 0b000000, isa)));
  instructions.push_back(
      std::shared_ptr<_Instruction>(new RVInstrIShift64Type<Reg_T>(
          Token("srli"), RVISA::OPIMM, 0b101, 0b000000, isa)));
  instructions.push_back(
      std::shared_ptr<_Instruction>(new RVInstrIShift64Type<Reg_T>(
          Token("srai"), RVISA::OPIMM, 0b101, 0b010000, isa)));

  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("addw"), 0b000, 0b0000000, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("subw"), 0b000, 0b0100000, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("sllw"), 0b001, 0b0000000, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("srlw"), 0b101, 0b0000000, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("sraw"), 0b101, 0b0100000, isa)));

  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrLType<Reg_T>(Token("lwu"), 0b110, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrLType<Reg_T>(Token("ld"), 0b011, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrSType<Reg_T>(Token("sd"), 0b011, isa)));

  pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
      new RVPseudoInstrLoad<Reg_T>(Token("ld"))));
  pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
      new RVPseudoInstrStore<Reg_T>(Token("sd"))));
  pseudoInstructions.push_back(
      std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
          Token("negw"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("subw"), line.tokens.at(1),
                                            Token("x0"), line.tokens.at(2)}};
          })));

  pseudoInstructions.push_back(
      std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
          Token("sext.w"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("addiw"), line.tokens.at(1),
                                            line.tokens.at(2), Token("0x0")}};
          })));
}

void RV64I_Assembler::enableExtM(const ISAInfoBase *isa,
                                 _InstrVec &instructions,
                                 _PseudoInstrVec &pseudoInstructions) {
  RV_M<Reg_T>::enable(isa, instructions, pseudoInstructions);

  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("mulw"), 0b000, 0b0000001, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("divw"), 0b100, 0b0000001, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("divuw"), 0b101, 0b0000001, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("remw"), 0b110, 0b0000001, isa)));
  instructions.push_back(std::shared_ptr<_Instruction>(
      new RVInstrR32Type<Reg_T>(Token("remuw"), 0b111, 0b0000001, isa)));
}

} // namespace Assembler
} // namespace Ripes
