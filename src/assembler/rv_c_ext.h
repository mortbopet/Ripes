#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "instruction.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

/// A base class for all RV-C registers (5-bit and 3-bit)
template <typename Reg_T>
class RVCReg : public RVReg<Reg_T> {
public:
  RVCReg(const ISAInfoBase *isa, unsigned tokenIndex, unsigned start,
         unsigned stop, const QString &regsd)
      : RVReg<Reg_T>(isa, tokenIndex, start, stop, regsd) {}
};

/// Register specialization for RV-C to handle the compressed register notation.
/// 3 bits are used to represent registers x8-x15, with register x8=0b000,
/// x15=0b111.
template <typename Reg_T>
struct RVCRegCompressed : public RVCReg<Reg_T> {
  RVCRegCompressed(const ISAInfoBase *isa, unsigned tokenIndex, unsigned start,
                   unsigned stop, const QString &regsd)
      : RVCReg<Reg_T>(isa, tokenIndex, start, stop, regsd) {}

  std::optional<Error> apply(const TokenizedSrcLine &line, Instr_T &instruction,
                             FieldLinkRequest<Reg_T> &) const override {
    bool success;
    const QString &regToken = line.tokens[this->tokenIndex];
    unsigned reg = this->m_isa->regNumber(regToken, success);
    if (!success)
      return Error(line, "Unknown register '" + regToken + "'");
    if (!(8 <= reg && reg <= 15))
      return Error(line,
                   "Only registers x8-x15 are allowed for this instruction");
    reg -= 8;
    instruction |= this->m_range.apply(reg);
    return std::nullopt;
  }

  std::optional<Error> decode(const Instr_T instruction,
                              const Reg_T /*address*/, const ReverseSymbolMap &,
                              LineTokens &line) const override {
    const unsigned regNumber = this->m_range.decode(instruction) + 8;
    const Token registerName = this->m_isa->regName(regNumber);
    if (registerName.isEmpty()) {
      return Error(0, "Unknown register number '" + QString::number(regNumber) +
                          "'");
    }
    line.push_back(registerName);
    return std::nullopt;
  }
};

// A base class for RISC-V compressed opcode parts
class RVCOpPart : public RVOpPart {
public:
  RVCOpPart(unsigned funct2, unsigned start, unsigned stop)
      : RVOpPart(funct2, start, stop) {}
};

// All RISC-V Funct2 opcode parts are defined as bits 5-6 (inclusive) of the
// instruction
class RVCOpPartFunct2 : public RVCOpPart {
public:
  RVCOpPartFunct2(unsigned funct2) : RVCOpPart(funct2, 5, 6) {}
};

// All RISC-V Funct3 opcode parts are defined as bits 13-15 (inclusive) of the
// instruction
class RVCOpPartFunct3 : public RVCOpPart {
public:
  RVCOpPartFunct3(unsigned funct3) : RVCOpPart(funct3, 13, 15) {}
};

// All RISC-V Funct4 opcode parts are defined as bits 12-15 (inclusive) of the
// instruction
class RVCOpPartFunct4 : public RVCOpPart {
public:
  RVCOpPartFunct4(unsigned funct4) : RVCOpPart(funct4, 12, 15) {}
};

// All RISC-V compressed Funct6 opcode parts are defined as bits 10-15
// (inclusive) of the instruction
class RVCOpPartFunct6 : public RVCOpPart {
public:
  RVCOpPartFunct6(unsigned funct6) : RVCOpPart(funct6, 10, 15) {}
};

// Bits 2-15 (inclusive) are set to 0 in a compressed NOP instruction
class RVCOpPartNOP : public RVCOpPart {
public:
  RVCOpPartNOP() : RVCOpPart(0, 2, 15) {}
};

// A RISC-V compressed opcode defines the encoding of specific compressed
// instructions
template <typename Reg_T>
class RVCOpcode : public RVOpcode<Reg_T> {
public:
  // A RISC-V compressed NOP opcode
  RVCOpcode(const Token &name, RVISA::Quadrant quadrant, RVCOpPartNOP nopPart)
      : RVOpcode<Reg_T>(name, {RVOpPartQuadrant(quadrant), nopPart}) {}

  // A RISC-V opcode with compressed Funct2 and Funct6 parts
  RVCOpcode(const Token &name, RVISA::Quadrant quadrant, RVCOpPartFunct2 funct2,
            RVCOpPartFunct6 funct6)
      : RVOpcode<Reg_T>(name, {RVOpPartQuadrant(quadrant), funct2, funct6}) {}

  // A RISC-V opcode with a compressed Funct3 part
  RVCOpcode(const Token &name, RVISA::Quadrant quadrant, RVCOpPartFunct3 funct3)
      : RVOpcode<Reg_T>(name, {RVOpPartQuadrant(quadrant), funct3}) {}

  // A RISC-V opcode with a compressed Funct4 part
  RVCOpcode(const Token &name, RVISA::Quadrant quadrant, RVCOpPartFunct4 funct4)
      : RVOpcode<Reg_T>(name, {RVOpPartQuadrant(quadrant), funct4}) {}
};

// A base class for RISC-V compressed instructions
template <typename Reg_T>
class RVCInstruction : public RVInstructionBase<Reg_T> {
public:
  RVCInstruction(const RVCOpcode<Reg_T> &opcode,
                 const std::vector<std::shared_ptr<Field<Reg_T>>> &fields)
      : RVInstructionBase<Reg_T>(opcode, fields) {}
};

// The RISC-V Compressed Rs1 field contains a source register index.
// It is defined as bits 7-11 (inclusive)
template <typename Reg_T>
class RVCRegRs1 : public RVCReg<Reg_T> {
public:
  RVCRegRs1(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCReg<Reg_T>(isa, fieldIndex, 7, 11, "rs1") {}
};

// The RISC-V Compressed Rs2 field contains a source register index.
// It is defined as bits 2-6 (inclusive)
template <typename Reg_T>
class RVCRegRs2 : public RVCReg<Reg_T> {
public:
  RVCRegRs2(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCReg<Reg_T>(isa, fieldIndex, 2, 6, "rs2") {}
};

// The RISC-V Compressed Rd/Rs1 field contains a source or destination register
// index. It is defined as bits 7-11 (inclusive)
template <typename Reg_T>
class RVCRegRdRs1 : public RVCReg<Reg_T> {
public:
  RVCRegRdRs1(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCReg<Reg_T>(isa, fieldIndex, 7, 11, "rd/rs1") {}
};

// The RISC-V Compressed Rs1 field contains a source register index.
// It is defined as bits 7-9 (inclusive)
template <typename Reg_T>
class RVCRegRs1Prime : public RVCRegCompressed<Reg_T> {
public:
  RVCRegRs1Prime(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCRegCompressed<Reg_T>(isa, fieldIndex, 7, 9, "rs1'") {}
};

// The RISC-V Compressed Rs2' field contains a source register index.
// It is defined as bits 2-4 (inclusive)
template <typename Reg_T>
class RVCRegRs2Prime : public RVCRegCompressed<Reg_T> {
public:
  RVCRegRs2Prime(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCRegCompressed<Reg_T>(isa, fieldIndex, 2, 4, "rs2'") {}
};

// The RISC-V Compressed Rd' field contains a destination register
// index. It is defined as bits 2-4 (inclusive)
template <typename Reg_T>
class RVCRegRdPrime : public RVCRegCompressed<Reg_T> {
public:
  RVCRegRdPrime(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCRegCompressed<Reg_T>(isa, fieldIndex, 2, 4, "rd'") {}
};

// The RISC-V Compressed Rd'/Rs1' field contains a source or destination
// register index. It is defined as bits 7-9 (inclusive)
template <typename Reg_T>
class RVCRegRdRs1Prime : public RVCRegCompressed<Reg_T> {
public:
  RVCRegRdRs1Prime(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCRegCompressed<Reg_T>(isa, fieldIndex, 7, 9, "rd'/rs1'") {}
};

// A base class for RISC-V compressed immediates
template <typename Reg_T>
class RVCImm : public RVImm<Reg_T> {
public:
  RVCImm(
      unsigned tokenIndex, unsigned width, typename Imm<Reg_T>::Repr repr,
      const std::vector<ImmPart> &parts,
      typename Imm<Reg_T>::SymbolType symbolType = Imm<Reg_T>::SymbolType::None)
      : RVImm<Reg_T>(tokenIndex, width, repr, parts, symbolType) {}
};

// A RISC-V unsigned immediate field with an input width of 8 bits.
// Used in C.LSWP and C.FLWSP instructions.
//
// It is defined as:
//  - Imm[7:6] = Inst[3:2]
//  - Imm[5]   = Inst[12]
//  - Imm[4:2] = Inst[6:4]
//  - Imm[1:0] = 0
template <typename Reg_T>
class RVCImmLWSP : public RVCImm<Reg_T> {
public:
  RVCImmLWSP()
      : RVCImm<Reg_T>(2, 8, Imm<Reg_T>::Repr::Unsigned,
                      std::vector{ImmPart(6, 2, 3), ImmPart(5, 12, 12),
                                  ImmPart(2, 4, 6)}) {}
};

// A RISC-V unsigned immediate field with an input width of 9 bits.
// Used in C.LDWP and C.FLDSP instructions.
//
// It is defined as:
//  - Imm[8:6] = Inst[4:2]
//  - Imm[5]   = Inst[12]
//  - Imm[4:3] = Inst[6:5]
//  - Imm[2:0] = 0
template <typename Reg_T>
class RVCImmLDSP : public RVCImm<Reg_T> {
public:
  RVCImmLDSP()
      : RVCImm<Reg_T>(2, 9, Imm<Reg_T>::Repr::Unsigned,
                      std::vector{ImmPart(6, 2, 4), ImmPart(5, 12, 12),
                                  ImmPart(3, 5, 6)}) {}
};

// A RISC-V immediate field with an input width of 6 bits.
// Used in the following instructions:
//  - C.ADDI (signed)
//  - C.ADDIW (signed)
//  - C.SLLI (unsigned)
//  - C.LI (signed)
//
// It is defined as:
//  - Imm[5]   = Inst[12]
//  - Imm[4:0] = Inst[6:2]
template <typename Reg_T>
class RVCImmCommon6 : public RVCImm<Reg_T> {
public:
  RVCImmCommon6(typename Imm<Reg_T>::Repr repr)
      : RVCImm<Reg_T>(2, 6, repr,
                      std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)}) {}
};

// A RISC-V signed immediate field with an input width of 18 bits.
// Used in C.LUI instructions.
//
// It is defined as:
//  - Imm[17]    = Inst[12]
//  - Imm[16:12] = Inst[6:2]
//  - Imm[12:0]  = 0
template <typename Reg_T>
class RVCImmLUI : public RVCImm<Reg_T> {
public:
  RVCImmLUI()
      : RVCImm<Reg_T>(2, 18, Imm<Reg_T>::Repr::Signed,
                      std::vector{ImmPart(17, 12, 12), ImmPart(12, 2, 6)}) {}
};

// A RISC-V unsigned immediate field with an input width of 8 bits.
// Used in C.SWSP and C.FSWSP instructions.
//
// It is defined as:
//  - Imm[7:6] = Inst[8:7]
//  - Imm[5:2] = Inst[12:9]
//  - Imm[1:0] = 0
template <typename Reg_T>
class RVCImmSWSP : public RVCImm<Reg_T> {
public:
  RVCImmSWSP()
      : RVCImm<Reg_T>(2, 8, Imm<Reg_T>::Repr::Unsigned,
                      std::vector{ImmPart(6, 7, 8), ImmPart(2, 9, 12)}) {}
};

// A RISC-V unsigned immediate field with an input width of 9 bits.
// Used in C.SDSP and C.FSDSP instructions.
//
// It is defined as:
//  - Imm[8:6] = Inst[9:7]
//  - Imm[5:3] = Inst[12:10]
//  - Imm[2:0] = 0
template <typename Reg_T>
class RVCImmSDSP : public RVCImm<Reg_T> {
public:
  RVCImmSDSP()
      : RVCImm<Reg_T>(2, 9, Imm<Reg_T>::Repr::Unsigned,
                      std::vector{ImmPart(6, 7, 9), ImmPart(3, 10, 12)}) {}
};

// A RISC-V immediate field with an input width of 7 bits.
// Used in the following instructions:
//  - C.LW
//  - C.FLW
//  - C.SW
//  - C.FSW
//  - C.SD
//  - C.FSD
//
// It is defined as:
//  - Imm[6]   = Inst[5]
//  - Imm[5:3] = Inst[12:10]
//  - Imm[2]   = Inst[6]
//  - Imm[1:0] = 0
template <typename Reg_T>
class RVCImmCommon7 : public RVCImm<Reg_T> {
public:
  RVCImmCommon7(typename Imm<Reg_T>::Repr repr)
      : RVCImm<Reg_T>(3, 7, repr,
                      std::vector{ImmPart(6, 5, 5), ImmPart(3, 10, 12),
                                  ImmPart(2, 6, 6)}) {}
};

// A RISC-V signed immediate field with an input width of 8 bits.
// Used in C.LD and C.FLD instructions.
//
// It is defined as:
//  - Imm[7:6] = Inst[6:5]
//  - Imm[5:3] = Inst[12:10]
//  - Imm[2:0] = 0
template <typename Reg_T>
class RVCImmLD : public RVCImm<Reg_T> {
public:
  RVCImmLD()
      : RVCImm<Reg_T>(3, 8, Imm<Reg_T>::Repr::Signed,
                      std::vector{ImmPart(6, 5, 6), ImmPart(3, 10, 12)}) {}
};

// A RISC-V signed immediate field with an input width of 12 bits.
// Used in C.J and C.JAL instructions.
//
// It is defined as:
//  - Imm[11]  = Inst[12]
//  - Imm[10]  = Inst[8]
//  - Imm[9:8] = Inst[10:9]
//  - Imm[7]   = Inst[6]
//  - Imm[6]   = Inst[7]
//  - Imm[5]   = Inst[2]
//  - Imm[4]   = Inst[11]
//  - Imm[3:1] = Inst[5:3]
//  - Imm[0]   = 0
template <typename Reg_T>
class RVCImmJ : public RVCImm<Reg_T> {
public:
  RVCImmJ()
      : RVCImm<Reg_T>(1, 12, Imm<Reg_T>::Repr::Signed,
                      std::vector{ImmPart(11, 12, 12), ImmPart(10, 8, 8),
                                  ImmPart(8, 9, 10), ImmPart(7, 6, 6),
                                  ImmPart(6, 7, 7), ImmPart(5, 2, 2),
                                  ImmPart(4, 11, 11), ImmPart(1, 3, 5)}) {}
};

template <typename Reg_T>
class CATypeInstr : public RVCInstruction<Reg_T> {
public:
  CATypeInstr(const Token &name, unsigned funct2, unsigned funct6,
              const ISAInfoBase *isa)
      : RVCInstruction<Reg_T>(
            RVCOpcode<Reg_T>(name, RVISA::Quadrant::QUADRANT1,
                             RVCOpPartFunct2(funct2), RVCOpPartFunct6(funct6)),
            {std::make_shared<RVCRegRs2Prime<Reg_T>>(isa, 2),
             std::make_shared<RVCRegRdRs1Prime<Reg_T>>(isa, 1)}) {}
};

template <typename Reg_T>
class CITypeInstr : public RVCInstruction<Reg_T> {
public:
  CITypeInstr(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
              const RVCImm<Reg_T> &imm, const ISAInfoBase *isa)
      : RVCInstruction<Reg_T>(
            RVCOpcode<Reg_T>(name, quadrant, RVCOpPartFunct3(funct3)),
            {std::make_shared<RVCRegRdRs1<Reg_T>>(isa, 1),
             std::make_shared<RVCImm<Reg_T>>(imm)}) {}
};

template <typename Reg_T>
class CINOPTypeInstr : public RVCInstruction<Reg_T> {
public:
  CINOPTypeInstr(RVISA::Quadrant quadrant, const Token &name)
      : RVCInstruction<Reg_T>(RVCOpcode<Reg_T>(name, quadrant, RVCOpPartNOP()),
                              {}) {}
};

template <typename Reg_T>
class CSSTypeInstr : public RVCInstruction<Reg_T> {
public:
  CSSTypeInstr(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
               const RVCImm<Reg_T> &imm, const ISAInfoBase *isa)
      : RVCInstruction<Reg_T>(
            RVCOpcode<Reg_T>(name, quadrant, RVCOpPartFunct3(funct3)),
            {std::make_shared<RVCRegRs2<Reg_T>>(isa, 1),
             std::make_shared<RVCImm<Reg_T>>(imm)}) {}
};

template <typename Reg_T>
class CLTypeInstr : public RVCInstruction<Reg_T> {
public:
  CLTypeInstr(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
              const RVCImm<Reg_T> &imm, const ISAInfoBase *isa)
      : RVCInstruction<Reg_T>(
            RVCOpcode<Reg_T>(name, quadrant, RVCOpPartFunct3(funct3)),
            {std::make_shared<RVCRegRdPrime<Reg_T>>(isa, 1),
             std::make_shared<RVCRegRs1Prime<Reg_T>>(isa, 2),
             std::make_shared<RVCImm<Reg_T>>(imm)}) {}
};

template <typename Reg_T>
class CSTypeInstr : public RVCInstruction<Reg_T> {
public:
  CSTypeInstr(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
              const ISAInfoBase *isa)
      : RVCInstruction<Reg_T>(
            RVCOpcode<Reg_T>(name, quadrant, RVCOpPartFunct3(funct3)),
            {std::make_shared<RVCRegRs2Prime<Reg_T>>(isa, 1),
             std::make_shared<RVCRegRs1Prime<Reg_T>>(isa, 2),
             std::make_shared<RVCImmCommon7<Reg_T>>(
                 Imm<Reg_T>::Repr::Unsigned)}) {}
};

template <typename Reg_T>
class CJTypeInstr : public RVCInstruction<Reg_T> {
public:
  CJTypeInstr(RVISA::Quadrant quadrant, const Token &name, unsigned funct3)
      : RVCInstruction<Reg_T>(
            RVCOpcode<Reg_T>(name, quadrant, RVCOpPartFunct3(funct3)),
            {std::make_shared<RVCImmJ<Reg_T>>()}) {}
};

template <typename Reg_T>
class CRTypeInstr : public RVCInstruction<Reg_T> {
public:
  CRTypeInstr(RVISA::Quadrant quadrant, const Token &name, unsigned funct4,
              const ISAInfoBase *isa)
      : RVCInstruction<Reg_T>(
            RVCOpcode<Reg_T>(name, quadrant, RVCOpPartFunct4(funct4)),
            {std::make_shared<RVCRegRs1<Reg_T>>(isa, 1),
             std::make_shared<RVCRegRs2<Reg_T>>(isa, 2)}) {}
};

#define CR2Type(opcode, name, funct4)                                          \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(0, 2, 6),             \
                            OpPart(funct4, 12, 15)}),                          \
      {std::make_shared<_Reg>(isa, 1, 7, 11, "rs1")}))

#define CREBREAKType(opcode, name, funct4)                                     \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(0, 2, 11),            \
                            OpPart(funct4, 12, 15)}),                          \
      {}))

#define CBType(opcode, name, funct3)                                           \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct3, 13, 15)}),    \
      {std::make_shared<RVCReg<Reg__T>>(isa, 1, 7, 9, "rs1'"),                 \
       std::make_shared<_Imm>(                                                 \
           2, 9, _Imm::Repr::Signed,                                           \
           std::vector{ImmPart(8, 12, 12), ImmPart(6, 5, 6), ImmPart(5, 2, 2), \
                       ImmPart(3, 10, 11), ImmPart(1, 3, 4)})}))

#define CB2Type(opcode, name, funct3, funct4, imm)                             \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct4, 10, 11),      \
                            OpPart(funct3, 13, 15)}),                          \
      {std::make_shared<RVCReg<Reg__T>>(isa, 1, 7, 9, "rs1'"), imm}))

#define CIWType(opcode, name, funct3)                                          \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct3, 13, 15)}),    \
      {std::make_shared<RVCReg<Reg__T>>(isa, 1, 2, 4, "rd'"),                  \
       std::make_shared<_Imm>(                                                 \
           2, 10, _Imm::Repr::Unsigned,                                        \
           std::vector{ImmPart(6, 7, 10), ImmPart(4, 11, 12),                  \
                       ImmPart(3, 5, 5), ImmPart(2, 6, 6)})}))

/**
 * Extension enabler.
 * Calling an extension enabler will register the appropriate assemblers
 * and pseudo-op expander functors with the assembler. The extension
 * enablers are templated to allow for sharing implementations between 32-
 * and 64-bit variants.
 */
template <typename Reg__T>
struct RV_C {
  AssemblerTypes(Reg__T);
  static void enable(const ISAInfoBase *isa, _InstrVec &instructions,
                     _PseudoInstrVec & /*pseudoInstructions*/) {
    // Pseudo-op functors

    // Assembler functors
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CATypeInstr<Reg__T>(Token("c.sub"), 0b00, 0b100011, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CATypeInstr<Reg__T>(Token("c.xor"), 0b01, 0b100011, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CATypeInstr<Reg__T>(Token("c.or"), 0b10, 0b100011, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CATypeInstr<Reg__T>(Token("c.and"), 0b11, 0b100011, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CATypeInstr<Reg__T>(Token("c.subw"), 0b00, 0b100111, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CATypeInstr<Reg__T>(Token("c.addw"), 0b01, 0b100111, isa)));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new CITypeInstr<Reg__T>(RVISA::Quadrant::QUADRANT2, Token("c.lwsp"),
                                0b010, RVCImmLWSP<Reg__T>(), isa)));

    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(std::shared_ptr<_Instruction>(
          new CITypeInstr<Reg__T>(RVISA::Quadrant::QUADRANT2, Token("c.flwsp"),
                                  0b011, RVCImmLWSP<Reg__T>(), isa)));
    } else // RV64 RV128
    {
      instructions.push_back(std::shared_ptr<_Instruction>(
          new CITypeInstr(RVISA::Quadrant::QUADRANT2, Token("c.ldsp"), 0b011,
                          RVCImmLDSP<Reg__T>(), isa)));
      instructions.push_back(std::shared_ptr<_Instruction>(
          new CITypeInstr(RVISA::Quadrant::QUADRANT1, Token("c.addiw"), 0b001,
                          RVCImmCommon6<Reg__T>(_Imm::Repr::Signed), isa)));
    }

    // instructions.push_back(CIType(0b10, Token("c.lqsp"), 0b001));//RV128
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CITypeInstr(RVISA::Quadrant::QUADRANT2, Token("c.fldsp"), 0b001,
                        RVCImmLDSP<Reg__T>(), isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CITypeInstr(RVISA::Quadrant::QUADRANT2, Token("c.slli"), 0b000,
                        RVCImmCommon6<Reg__T>(_Imm::Repr::Unsigned), isa)));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new CITypeInstr(RVISA::Quadrant::QUADRANT1, Token("c.li"), 0b010,
                        RVCImmCommon6<Reg__T>(_Imm::Repr::Signed), isa)));

    auto cLuiInstr = std::shared_ptr<_Instruction>(
        new CITypeInstr(RVISA::Quadrant::QUADRANT1, Token("c.lui"), 0b011,
                        RVCImmLUI<Reg__T>(), isa));
    cLuiInstr->addExtraMatchCond([](Instr_T instr) {
      unsigned rd = (instr >> 7) & 0b11111;
      return rd != 0 && rd != 2;
    });

    instructions.push_back(cLuiInstr);

    auto cAddi16spInstr = std::shared_ptr<_Instruction>(new _Instruction(
        Opcode<Reg__T>(
            Token("c.addi16sp"),
            {OpPart(0b01, 0, 1), OpPart(0b011, 13, 15), OpPart(2, 7, 11)}),
        {std::make_shared<_Imm>(
            1, 10, _Imm::Repr::Signed,
            std::vector{ImmPart(9, 12, 12), ImmPart(7, 3, 4), ImmPart(6, 5, 5),
                        ImmPart(5, 2, 2), ImmPart(4, 6, 6)})}));
    cAddi16spInstr->addExtraMatchCond([](Instr_T instr) {
      unsigned rd = (instr >> 7) & 0b11111;
      return rd == 2;
    });
    instructions.push_back(cAddi16spInstr);

    instructions.push_back(std::shared_ptr<_Instruction>(
        new CITypeInstr(RVISA::Quadrant::QUADRANT1, Token("c.addi"), 0b000,
                        RVCImmCommon6<Reg__T>(_Imm::Repr::Signed), isa)));
    instructions.push_back(
        std::shared_ptr<_Instruction>(new CINOPTypeInstr<Reg__T>(
            RVISA::Quadrant::QUADRANT1, Token("c.nop"))));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new CSSTypeInstr(RVISA::Quadrant::QUADRANT2, Token("c.swsp"), 0b110,
                         RVCImmSWSP<Reg__T>(), isa)));
    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(std::shared_ptr<_Instruction>(
          new CSSTypeInstr(RVISA::Quadrant::QUADRANT2, Token("c.fswsp"), 0b111,
                           RVCImmSWSP<Reg__T>(), isa)));
    } else {
      instructions.push_back(std::shared_ptr<_Instruction>(
          new CSSTypeInstr(RVISA::Quadrant::QUADRANT2, Token("c.sdsp"), 0b111,
                           RVCImmSDSP<Reg__T>(), isa)));
    }
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CSSTypeInstr(RVISA::Quadrant::QUADRANT2, Token("c.fsdsp"), 0b101,
                         RVCImmSDSP<Reg__T>(), isa)));
    // instructions.push_back(CSSType(0b10, Token("c.sqsp"), 0b101));//RV128

    instructions.push_back(std::shared_ptr<_Instruction>(new CLTypeInstr(
        RVISA::Quadrant::QUADRANT0, Token("c.lw"), 0b010,
        RVCImmCommon7<Reg__T>(Imm<Reg__T>::Repr::Signed), isa)));
    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(std::shared_ptr<_Instruction>(new CLTypeInstr(
          RVISA::Quadrant::QUADRANT0, Token("c.flw"), 0b011,
          RVCImmCommon7<Reg__T>(Imm<Reg__T>::Repr::Signed), isa)));
    } else {
      instructions.push_back(std::shared_ptr<_Instruction>(
          new CLTypeInstr(RVISA::Quadrant::QUADRANT0, Token("c.ld"), 0b011,
                          RVCImmLD<Reg__T>(), isa)));
    }
    // instructions.push_back(CLType(0b00, Token("c.lq"), 0b001));//RV128
    instructions.push_back(std::shared_ptr<_Instruction>(
        new CLTypeInstr(RVISA::Quadrant::QUADRANT0, Token("c.fld"), 0b001,
                        RVCImmLD<Reg__T>(), isa)));

    instructions.push_back(
        std::shared_ptr<_Instruction>(new CSTypeInstr<Reg__T>(
            RVISA::Quadrant::QUADRANT0, Token("c.sw"), 0b110, isa)));
    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(
          std::shared_ptr<_Instruction>(new CSTypeInstr<Reg__T>(
              RVISA::Quadrant::QUADRANT0, Token("c.fsw"), 0b111, isa)));
    } else {
      instructions.push_back(
          std::shared_ptr<_Instruction>(new CSTypeInstr<Reg__T>(
              RVISA::Quadrant::QUADRANT0, Token("c.sd"), 0b111, isa)));
    }
    // instructions.push_back(CSType(0b00, Token("c.sq"), 0b101));//RV128
    instructions.push_back(
        std::shared_ptr<_Instruction>(new CSTypeInstr<Reg__T>(
            RVISA::Quadrant::QUADRANT0, Token("c.fsd"), 0b101, isa)));

    instructions.push_back(
        std::shared_ptr<_Instruction>(new CJTypeInstr<Reg__T>(
            RVISA::Quadrant::QUADRANT1, Token("c.j"), 0b101)));
    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(
          std::shared_ptr<_Instruction>(new CJTypeInstr<Reg__T>(
              RVISA::Quadrant::QUADRANT1, Token("c.jal"), 0b001)));
    }

    instructions.push_back(CBType(0b01, Token("c.beqz"), 0b110));
    instructions.push_back(CBType(0b01, Token("c.bnez"), 0b111));

    instructions.push_back(CIWType(0b00, Token("c.addi4spn"), 0b000));

    instructions.push_back(
        CB2Type(0b01, Token("c.srli"), 0b100, 0b00,
                std::make_shared<_Imm>(
                    2, 6, _Imm::Repr::Unsigned,
                    std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));
    instructions.push_back(
        CB2Type(0b01, Token("c.srai"), 0b100, 0b01,
                std::make_shared<_Imm>(
                    2, 6, _Imm::Repr::Unsigned,
                    std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));

    instructions.push_back(
        CB2Type(0b01, Token("c.andi"), 0b100, 0b10,
                std::make_shared<_Imm>(
                    2, 6, _Imm::Repr::Signed,
                    std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new CRTypeInstr<Reg__T>(RVISA::Quadrant::QUADRANT2, Token("c.mv"),
                                0b1000,
                                isa))); // FIXME disassemble erro with c.jr ?
    instructions.push_back(
        std::shared_ptr<_Instruction>(new CRTypeInstr<Reg__T>(
            RVISA::Quadrant::QUADRANT2, Token("c.add"), 0b1001, isa)));

    instructions.push_back(CR2Type(0b10, Token("c.jr"), 0b1000));
    instructions.push_back(CR2Type(0b10, Token("c.jalr"), 0b1001));

    // instructions.push_back(CREBREAKType(0b10, Token("c.ebreak"), 0b1001));
    // //FIXME Duplicated terminate called after throwing an instance of
    // 'std::runtime_error' what():  Instruction cannot be decoded; aliases with
    // other instruction (Identical to other instruction) c.ebreak is equal to
    // c.jalr
  }
};

} // namespace Assembler
} // namespace Ripes
