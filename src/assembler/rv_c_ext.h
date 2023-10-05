#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "instruction.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

/// A base class for all RV-C registers (5-bit and 3-bit)
struct RVCReg : public RVReg {
  RVCReg(const ISAInfoBase *isa, unsigned tokenIndex, unsigned start,
         unsigned stop, const QString &regsd)
      : RVReg(isa, tokenIndex, start, stop, regsd) {}
};

/// A base class for RV-C 3-bit registers
///
/// Register specialization for RV-C to handle the compressed register notation.
/// 3 bits are used to represent registers x8-x15, with register x8=0b000,
/// x15=0b111.
struct RVCRegCompressed : public RVCReg {
  RVCRegCompressed(const ISAInfoBase *isa, unsigned tokenIndex, unsigned start,
                   unsigned stop, const QString &regsd)
      : RVCReg(isa, tokenIndex, start, stop, regsd) {}

  std::optional<Error> apply(const TokenizedSrcLine &line, Instr_T &instruction,
                             FieldLinkRequest &) const override {
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

/// A base class for RV-C opcode parts
struct RVCOpPart : public RVOpPart {
  RVCOpPart(unsigned value, unsigned start, unsigned stop)
      : RVOpPart(value, start, stop) {}
};

/// All RISC-V Funct2 opcode parts are defined as a 2-bit field in bits 5-6 of
/// the instruction
struct RVCOpPartFunct2 : public RVCOpPart {
  enum Offset { OFFSET5 = 5, OFFSET10 = 10 };
  RVCOpPartFunct2(unsigned funct2, Offset offset)
      : RVCOpPart(funct2, offset, offset + 1) {}
};

/// All RISC-V Funct3 opcode parts are defined as a 3-bit field in bits 13-15 of
/// the instruction
struct RVCOpPartFunct3 : public RVCOpPart {
  RVCOpPartFunct3(unsigned funct3) : RVCOpPart(funct3, 13, 15) {}
};

/// All RISC-V Funct4 opcode parts are defined as a 4-bit field in bits 12-15 of
/// the instruction
struct RVCOpPartFunct4 : public RVCOpPart {
  RVCOpPartFunct4(unsigned funct4) : RVCOpPart(funct4, 12, 15) {}
};

struct RVCInstrCAType;
struct RVCInstrCINOPType;

/// A RV-C opcode defines the encoding of specific compressed
/// instructions
struct RVCOpcode : public RVOpcode {
  /// Construct opcode from parts
  RVCOpcode(const Token &name, const std::vector<OpPart> &opParts)
      : RVOpcode(name, opParts) {}

  /// An RV-C opcode with a Funct3 part
  RVCOpcode(const Token &name, RVISA::Quadrant quadrant, RVCOpPartFunct3 funct3)
      : RVOpcode(name, {RVOpPartQuadrant(quadrant), funct3}) {}

  /// An RV-C opcode with a Funct4 part
  RVCOpcode(const Token &name, RVISA::Quadrant quadrant, RVCOpPartFunct4 funct4)
      : RVOpcode(name, {RVOpPartQuadrant(quadrant), funct4}) {}

  /// An RV-C opcode with a Funct3 part
  RVCOpcode(const Token &name, RVISA::Quadrant quadrant, RVCOpPartFunct2 funct2,
            RVCOpPartFunct3 funct3)
      : RVOpcode(name, {RVOpPartQuadrant(quadrant), funct2, funct3}) {}
};

/// A base class for RV-C instructions
struct RVCInstruction : public RVInstructionBase {
  RVCInstruction(const RVCOpcode &opcode,
                 const std::vector<std::shared_ptr<Field>> &fields)
      : RVInstructionBase(opcode, fields) {}
};

/// The RV-C Rs1 field contains a source register index.
/// It is defined as a 5-bit field in bits 7-11 of the instruction
struct RVCRegRs1 : public RVCReg {
  RVCRegRs1(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCReg(isa, fieldIndex, 7, 11, "rs1") {}
};

/// The RV-C Rs2 field contains a source register index.
/// It is defined as a 5-bit field in bits 2-6 of the instruction
struct RVCRegRs2 : public RVCReg {
  RVCRegRs2(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCReg(isa, fieldIndex, 2, 6, "rs2") {}
};

/// The RV-C Rs1' field contains a source register index.
/// It is defined as a 3-bit field in bits 7-9 of the instruction
struct RVCRegRs1Prime : public RVCRegCompressed {
  RVCRegRs1Prime(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCRegCompressed(isa, fieldIndex, 7, 9, "rs1'") {}
};

/// The RV-C Rs2' field contains a source register index.
/// It is defined as a 3-bit field in bits 2-4 of the instruction
struct RVCRegRs2Prime : public RVCRegCompressed {
  RVCRegRs2Prime(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCRegCompressed(isa, fieldIndex, 2, 4, "rs2'") {}
};

/// The RV-C Rd' field contains a destination register
/// index.
/// It is defined as a 3-bit field in bits 2-4 of the instruction
struct RVCRegRdPrime : public RVCRegCompressed {
  RVCRegRdPrime(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVCRegCompressed(isa, fieldIndex, 2, 4, "rd'") {}
};

/// A base class for RV-C immediates
struct RVCImm : public RVImm {
  RVCImm(unsigned tokenIndex, unsigned width, typename Imm::Repr repr,
         const std::vector<ImmPart> &parts,
         typename Imm::SymbolType symbolType = Imm::SymbolType::None)
      : RVImm(tokenIndex, width, repr, parts, symbolType) {}
};

/// An RV-C immediate field with an input width of 6 bits.
/// Used in the following instructions:
///  - C.ADDI (signed)
///  - C.ADDIW (signed)
///  - C.SLLI (unsigned)
///  - C.LI (signed)
///
/// It is defined as:
///  - Imm[5]   = Inst[12]
///  - Imm[4:0] = Inst[6:2]
struct RVCImmCommon6 : public RVCImm {
  RVCImmCommon6(typename Imm::Repr repr)
      : RVCImm(2, 6, repr, std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)}) {}
};

/// An RV-C immediate field with an input width of 7 bits.
/// Used in the following instructions:
///  - C.LW
///  - C.FLW
///  - C.SW
///  - C.FSW
///  - C.SD
///  - C.FSD
///
/// It is defined as:
///  - Imm[6]   = Inst[5]
///  - Imm[5:3] = Inst[12:10]
///  - Imm[2]   = Inst[6]
///  - Imm[1:0] = 0
struct RVCImmCommon7 : public RVCImm {
  RVCImmCommon7(typename Imm::Repr repr)
      : RVCImm(3, 7, repr,
               std::vector{ImmPart(6, 5, 5), ImmPart(3, 10, 12),
                           ImmPart(2, 6, 6)}) {}
};

/// A CA-Type RV-C instruction
struct RVCInstrCAType : public RVCInstruction {
  RVCInstrCAType(const Token &name, unsigned funct2, unsigned funct6,
                 const ISAInfoBase *isa)
      : RVCInstruction(
            RVCOpcode(name, {RVOpPartQuadrant(RVISA::Quadrant::QUADRANT1),
                             RVCOpPartFunct2(funct2, RVCOpPartFunct2::OFFSET5),
                             OpPartFunct6(funct6)}),
            {std::make_shared<RVCRegRs2Prime>(isa, 2),
             std::make_shared<RegRdRs1Prime>(isa, 1)}) {}

  /// All RV-C Funct6 opcode parts are defined as a 6-bit field in bits 10-15
  /// of the instruction
  struct OpPartFunct6 : public RVCOpPart {
    OpPartFunct6(unsigned funct6) : RVCOpPart(funct6, 10, 15) {}
  };

  /// The RV-C Rd'/Rs1' field contains a source or destination
  /// register index.
  /// It is defined as a 3-bit field in bits 7-9 of the instruction
  struct RegRdRs1Prime : public RVCRegCompressed {
    RegRdRs1Prime(const ISAInfoBase *isa, unsigned fieldIndex)
        : RVCRegCompressed(isa, fieldIndex, 7, 9, "rd'/rs1'") {}
  };
};

/// A CI-Type RV-C instruction
struct RVCInstrCIType : public RVCInstruction {
  RVCInstrCIType(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
                 const RVCImm &imm, const ISAInfoBase *isa)
      : RVCInstruction(RVCOpcode(name, quadrant, RVCOpPartFunct3(funct3)),
                       {std::make_shared<RegRdRs1>(isa, 1),
                        std::make_shared<RVCImm>(imm)}) {}

  /// The RV-C Rd/Rs1 field contains a source or destination register
  /// index.
  /// It is defined as a 5-bit field in bits 7-11 of the instruction
  struct RegRdRs1 : public RVCReg {
    RegRdRs1(const ISAInfoBase *isa, unsigned fieldIndex)
        : RVCReg(isa, fieldIndex, 7, 11, "rd/rs1") {}
  };

  /// An RV-C unsigned immediate field with an input width of 8 bits.
  /// Used in C.LSWP and C.FLWSP instructions.
  ///
  /// It is defined as:
  ///  - Imm[7:6] = Inst[3:2]
  ///  - Imm[5]   = Inst[12]
  ///  - Imm[4:2] = Inst[6:4]
  ///  - Imm[1:0] = 0
  struct ImmLWSP : public RVCImm {
    ImmLWSP()
        : RVCImm(2, 8, Imm::Repr::Unsigned,
                 std::vector{ImmPart(6, 2, 3), ImmPart(5, 12, 12),
                             ImmPart(2, 4, 6)}) {}
  };

  /// An RV-C unsigned immediate field with an input width of 9 bits.
  /// Used in C.LDWP and C.FLDSP instructions.
  ///
  /// It is defined as:
  ///  - Imm[8:6] = Inst[4:2]
  ///  - Imm[5]   = Inst[12]
  ///  - Imm[4:3] = Inst[6:5]
  ///  - Imm[2:0] = 0
  struct ImmLDSP : public RVCImm {
    ImmLDSP()
        : RVCImm(2, 9, Imm::Repr::Unsigned,
                 std::vector{ImmPart(6, 2, 4), ImmPart(5, 12, 12),
                             ImmPart(3, 5, 6)}) {}
  };

  /// An RV-C signed immediate field with an input width of 18 bits.
  /// Used in C.LUI instructions.
  ///
  /// It is defined as:
  ///  - Imm[17]    = Inst[12]
  ///  - Imm[16:12] = Inst[6:2]
  ///  - Imm[12:0]  = 0
  struct ImmLUI : public RVCImm {
    ImmLUI()
        : RVCImm(2, 18, Imm::Repr::Signed,
                 std::vector{ImmPart(17, 12, 12), ImmPart(12, 2, 6)}) {}
  };
};

/// A CINOP-Type RV-C instruction
struct RVCInstrCINOPType : public RVCInstruction {
  RVCInstrCINOPType(RVISA::Quadrant quadrant, const Token &name)
      : RVCInstruction(
            RVCOpcode(name, {RVOpPartQuadrant(quadrant), OpPartNOP()}), {}) {}

  /// The 14-bit field from bits 2-15 are set to 0 in a compressed NOP
  /// instruction
  struct OpPartNOP : public RVCOpPart {
    OpPartNOP() : RVCOpPart(0, 2, 15) {}
  };
};

/// A CSS-Type RV-C instruction
struct RVCInstrCSSType : public RVCInstruction {
  RVCInstrCSSType(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
                  const RVCImm &imm, const ISAInfoBase *isa)
      : RVCInstruction(RVCOpcode(name, quadrant, RVCOpPartFunct3(funct3)),
                       {std::make_shared<RVCRegRs2>(isa, 1),
                        std::make_shared<RVCImm>(imm)}) {}

  /// An RV-C unsigned immediate field with an input width of 8 bits.
  /// Used in C.SWSP and C.FSWSP instructions.
  ///
  /// It is defined as:
  ///  - Imm[7:6] = Inst[8:7]
  ///  - Imm[5:2] = Inst[12:9]
  ///  - Imm[1:0] = 0
  struct ImmSWSP : public RVCImm {
    ImmSWSP()
        : RVCImm(2, 8, Imm::Repr::Unsigned,
                 std::vector{ImmPart(6, 7, 8), ImmPart(2, 9, 12)}) {}
  };

  /// An RV-C unsigned immediate field with an input width of 9 bits.
  /// Used in C.SDSP and C.FSDSP instructions.
  ///
  /// It is defined as:
  ///  - Imm[8:6] = Inst[9:7]
  ///  - Imm[5:3] = Inst[12:10]
  ///  - Imm[2:0] = 0
  struct ImmSDSP : public RVCImm {
    ImmSDSP()
        : RVCImm(2, 9, Imm::Repr::Unsigned,
                 std::vector{ImmPart(6, 7, 9), ImmPart(3, 10, 12)}) {}
  };
};

/// A CL-Type RV-C instruction
struct RVCInstrCLType : public RVCInstruction {
  RVCInstrCLType(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
                 const RVCImm &imm, const ISAInfoBase *isa)
      : RVCInstruction(RVCOpcode(name, quadrant, RVCOpPartFunct3(funct3)),
                       {std::make_shared<RVCRegRdPrime>(isa, 1),
                        std::make_shared<RVCRegRs1Prime>(isa, 2),
                        std::make_shared<RVCImm>(imm)}) {}

  /// An RV-C signed immediate field with an input width of 8 bits.
  /// Used in C.LD and C.FLD instructions.
  ///
  /// It is defined as:
  ///  - Imm[7:6] = Inst[6:5]
  ///  - Imm[5:3] = Inst[12:10]
  ///  - Imm[2:0] = 0
  struct ImmLD : public RVCImm {
    ImmLD()
        : RVCImm(3, 8, Imm::Repr::Signed,
                 std::vector{ImmPart(6, 5, 6), ImmPart(3, 10, 12)}) {}
  };
};

/// A CS-Type RV-C instruction
struct RVCInstrCSType : public RVCInstruction {
  RVCInstrCSType(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
                 const ISAInfoBase *isa)
      : RVCInstruction(RVCOpcode(name, quadrant, RVCOpPartFunct3(funct3)),
                       {std::make_shared<RVCRegRs2Prime>(isa, 1),
                        std::make_shared<RVCRegRs1Prime>(isa, 2),
                        std::make_shared<RVCImmCommon7>(Imm::Repr::Unsigned)}) {
  }
};

/// A CJ-Type RV-C instruction
struct RVCInstrCJType : public RVCInstruction {
  RVCInstrCJType(RVISA::Quadrant quadrant, const Token &name, unsigned funct3)
      : RVCInstruction(RVCOpcode(name, quadrant, RVCOpPartFunct3(funct3)),
                       {std::make_shared<ImmJ>()}) {}

  /// An RV-C signed immediate field with an input width of 12 bits.
  /// Used in C.J and C.JAL instructions.
  ///
  /// It is defined as:
  ///  - Imm[11]  = Inst[12]
  ///  - Imm[10]  = Inst[8]
  ///  - Imm[9:8] = Inst[10:9]
  ///  - Imm[7]   = Inst[6]
  ///  - Imm[6]   = Inst[7]
  ///  - Imm[5]   = Inst[2]
  ///  - Imm[4]   = Inst[11]
  ///  - Imm[3:1] = Inst[5:3]
  ///  - Imm[0]   = 0
  struct ImmJ : public RVCImm {
    ImmJ()
        : RVCImm(1, 12, Imm::Repr::Signed,
                 std::vector{ImmPart(11, 12, 12), ImmPart(10, 8, 8),
                             ImmPart(8, 9, 10), ImmPart(7, 6, 6),
                             ImmPart(6, 7, 7), ImmPart(5, 2, 2),
                             ImmPart(4, 11, 11), ImmPart(1, 3, 5)}) {}
  };
};

/// A CR-Type RV-C instruction
struct RVCInstrCRType : public RVCInstruction {
  RVCInstrCRType(RVISA::Quadrant quadrant, const Token &name, unsigned funct4,
                 const ISAInfoBase *isa)
      : RVCInstruction(RVCOpcode(name, quadrant, RVCOpPartFunct4(funct4)),
                       {std::make_shared<RVCRegRs1>(isa, 1),
                        std::make_shared<RVCRegRs2>(isa, 2)}) {}
};

/// A CR2-Type RV-C instruction
struct RVCInstrCR2Type : public RVCInstruction {
  RVCInstrCR2Type(RVISA::Quadrant quadrant, const Token &name, unsigned funct4,
                  const ISAInfoBase *isa)
      : RVCInstruction(
            RVCOpcode(name, {RVOpPartQuadrant(quadrant), RVOpPart(0, 2, 6),
                             RVCOpPartFunct4(funct4)}),
            {std::make_shared<RVCRegRs1>(isa, 1)}) {}
};

/// A CB-Type RV-C instruction
struct RVCInstrCBType : public RVCInstruction {
  RVCInstrCBType(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
                 const ISAInfoBase *isa)
      : RVCInstruction(RVCOpcode(name, quadrant, RVCOpPartFunct3(funct3)),
                       {std::make_shared<RVCRegRs1Prime>(isa, 1),
                        std::make_shared<ImmB>()}) {}

  /// An RV-C signed immediate field with an input width of 9 bits.
  /// Used in C.BEQZ and C.BNEZ instructions.
  ///
  /// It is defined as:
  ///  - Imm[8]   = Inst[12]
  ///  - Imm[7:6] = Inst[6:5]
  ///  - Imm[5]   = Inst[2]
  ///  - Imm[4:3] = Inst[11:10]
  ///  - Imm[2:1] = Inst[4:3]
  ///  - Imm[0]   = 0
  struct ImmB : public RVCImm {
    ImmB()
        : RVCImm(2, 9, Imm::Repr::Signed,
                 std::vector{ImmPart(8, 12, 12), ImmPart(6, 5, 6),
                             ImmPart(5, 2, 2), ImmPart(3, 10, 11),
                             ImmPart(1, 3, 4)}) {}
  };
};

/// A CB2-Type RV-C instruction
struct RVCInstrCB2Type : public RVCInstruction {
  RVCInstrCB2Type(RVISA::Quadrant quadrant, const Token &name, unsigned funct2,
                  unsigned funct3, typename Imm::Repr repr,
                  const ISAInfoBase *isa)
      : RVCInstruction(
            RVCOpcode(name, quadrant,
                      RVCOpPartFunct2(funct2, RVCOpPartFunct2::OFFSET10),
                      RVCOpPartFunct3(funct3)),
            {std::make_shared<RVCRegRs1Prime>(isa, 1),
             std::make_shared<ImmB2>(repr)}) {}

  /// An RV-C immediate field with an input width of 6 bits.
  /// Used in the following instructions:
  ///  - C.SRLI (unsigned)
  ///  - C.SRAI (unsigned)
  ///  - C.ANDI (signed)
  ///
  /// It is defined as:
  ///  - Imm[5]   = Inst[12]
  ///  - Imm[4:0] = Inst[6:2]
  struct ImmB2 : public RVCImm {
    ImmB2(typename Imm::Repr repr)
        : RVCImm(2, 6, repr,
                 std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)}) {}
  };
};

/// A CIW-Type RV-C instruction
struct RVCInstrCIWType : public RVCInstruction {
  RVCInstrCIWType(RVISA::Quadrant quadrant, const Token &name, unsigned funct3,
                  const ISAInfoBase *isa)
      : RVCInstruction(RVCOpcode(name, quadrant, RVCOpPartFunct3(funct3)),
                       {std::make_shared<RVCRegRdPrime>(isa, 1),
                        std::make_shared<ImmIW>()}) {}

  /// An RV-C unsigned immediate field with an input width of 10 bits.
  /// Used in the C.ADDI4SPN instruction.
  ///
  /// It is defined as:
  ///  - Imm[9:6] = Inst[10:7]
  ///  - Imm[5:4] = Inst[12:11]
  ///  - Imm[3]   = Inst[5]
  ///  - Imm[2]   = Inst[6]
  ///  - Imm[1:0] = 0
  struct ImmIW : public RVCImm {
    ImmIW()
        : RVCImm(2, 10, Imm::Repr::Unsigned,
                 std::vector{ImmPart(6, 7, 10), ImmPart(4, 11, 12),
                             ImmPart(3, 5, 5), ImmPart(2, 6, 6)}) {}
  };
};

#define CREBREAKType(opcode, name, funct4)                                     \
  std::shared_ptr<Instruction>(                                                \
      new Instruction(Opcode(name, {OpPart(opcode, 0, 1), OpPart(0, 2, 11),    \
                                    OpPart(funct4, 12, 15)}),                  \
                      {}))

/**
 * Extension enabler.
 * Calling an extension enabler will register the appropriate assemblers
 * and pseudo-op expander functors with the assembler.
 */
struct RV_C {
  static void enable(const ISAInfoBase *isa, InstrVec &instructions,
                     PseudoInstrVec & /*pseudoInstructions*/) {
    // Pseudo-op functors

    // Assembler functors
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCAType(Token("c.sub"), 0b00, 0b100011, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCAType(Token("c.xor"), 0b01, 0b100011, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCAType(Token("c.or"), 0b10, 0b100011, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCAType(Token("c.and"), 0b11, 0b100011, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCAType(Token("c.subw"), 0b00, 0b100111, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCAType(Token("c.addw"), 0b01, 0b100111, isa)));

    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCIType(RVISA::Quadrant::QUADRANT2, Token("c.lwsp"), 0b010,
                           typename RVCInstrCIType::ImmLWSP(), isa)));

    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(std::shared_ptr<Instruction>(
          new RVCInstrCIType(RVISA::Quadrant::QUADRANT2, Token("c.flwsp"),
                             0b011, typename RVCInstrCIType::ImmLWSP(), isa)));
    } else // RV64 RV128
    {
      instructions.push_back(std::shared_ptr<Instruction>(
          new RVCInstrCIType(RVISA::Quadrant::QUADRANT2, Token("c.ldsp"), 0b011,
                             typename RVCInstrCIType::ImmLDSP(), isa)));
      instructions.push_back(std::shared_ptr<Instruction>(
          new RVCInstrCIType(RVISA::Quadrant::QUADRANT1, Token("c.addiw"),
                             0b001, RVCImmCommon6(Imm::Repr::Signed), isa)));
    }

    // instructions.push_back(CIType(0b10, Token("c.lqsp"), 0b001));//RV128
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCIType(RVISA::Quadrant::QUADRANT2, Token("c.fldsp"), 0b001,
                           typename RVCInstrCIType::ImmLDSP(), isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCIType(RVISA::Quadrant::QUADRANT2, Token("c.slli"), 0b000,
                           RVCImmCommon6(Imm::Repr::Unsigned), isa)));

    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCIType(RVISA::Quadrant::QUADRANT1, Token("c.li"), 0b010,
                           RVCImmCommon6(Imm::Repr::Signed), isa)));

    auto cLuiInstr = std::shared_ptr<Instruction>(
        new RVCInstrCIType(RVISA::Quadrant::QUADRANT1, Token("c.lui"), 0b011,
                           typename RVCInstrCIType::ImmLUI(), isa));
    cLuiInstr->addExtraMatchCond([](Instr_T instr) {
      unsigned rd = (instr >> 7) & 0b11111;
      return rd != 0 && rd != 2;
    });

    instructions.push_back(cLuiInstr);

    auto cAddi16spInstr = std::shared_ptr<Instruction>(new Instruction(
        Opcode(Token("c.addi16sp"),
               {OpPart(0b01, 0, 1), OpPart(0b011, 13, 15), OpPart(2, 7, 11)}),
        {std::make_shared<Imm>(1, 10, Imm::Repr::Signed,
                               std::vector{ImmPart(9, 12, 12), ImmPart(7, 3, 4),
                                           ImmPart(6, 5, 5), ImmPart(5, 2, 2),
                                           ImmPart(4, 6, 6)})}));
    cAddi16spInstr->addExtraMatchCond([](Instr_T instr) {
      unsigned rd = (instr >> 7) & 0b11111;
      return rd == 2;
    });
    instructions.push_back(cAddi16spInstr);

    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCIType(RVISA::Quadrant::QUADRANT1, Token("c.addi"), 0b000,
                           RVCImmCommon6(Imm::Repr::Signed), isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCINOPType(RVISA::Quadrant::QUADRANT1, Token("c.nop"))));

    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCSSType(RVISA::Quadrant::QUADRANT2, Token("c.swsp"), 0b110,
                            typename RVCInstrCSSType::ImmSWSP(), isa)));
    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCSSType(
          RVISA::Quadrant::QUADRANT2, Token("c.fswsp"), 0b111,
          typename RVCInstrCSSType::ImmSWSP(), isa)));
    } else {
      instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCSSType(
          RVISA::Quadrant::QUADRANT2, Token("c.sdsp"), 0b111,
          typename RVCInstrCSSType::ImmSDSP(), isa)));
    }
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCSSType(RVISA::Quadrant::QUADRANT2, Token("c.fsdsp"), 0b101,
                            typename RVCInstrCSSType::ImmSDSP(), isa)));
    // instructions.push_back(CSSType(0b10, Token("c.sqsp"), 0b101));//RV128

    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCLType(RVISA::Quadrant::QUADRANT0, Token("c.lw"), 0b010,
                           RVCImmCommon7(Imm::Repr::Signed), isa)));
    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(std::shared_ptr<Instruction>(
          new RVCInstrCLType(RVISA::Quadrant::QUADRANT0, Token("c.flw"), 0b011,
                             RVCImmCommon7(Imm::Repr::Signed), isa)));
    } else {
      instructions.push_back(std::shared_ptr<Instruction>(
          new RVCInstrCLType(RVISA::Quadrant::QUADRANT0, Token("c.ld"), 0b011,
                             typename RVCInstrCLType::ImmLD(), isa)));
    }
    // instructions.push_back(CLType(0b00, Token("c.lq"), 0b001));//RV128
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCLType(RVISA::Quadrant::QUADRANT0, Token("c.fld"), 0b001,
                           typename RVCInstrCLType::ImmLD(), isa)));

    instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCSType(
        RVISA::Quadrant::QUADRANT0, Token("c.sw"), 0b110, isa)));
    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCSType(
          RVISA::Quadrant::QUADRANT0, Token("c.fsw"), 0b111, isa)));
    } else {
      instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCSType(
          RVISA::Quadrant::QUADRANT0, Token("c.sd"), 0b111, isa)));
    }
    // instructions.push_back(CSType(0b00, Token("c.sq"), 0b101));//RV128
    instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCSType(
        RVISA::Quadrant::QUADRANT0, Token("c.fsd"), 0b101, isa)));

    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCJType(RVISA::Quadrant::QUADRANT1, Token("c.j"), 0b101)));
    if (isa->isaID() == ISA::RV32I) {
      instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCJType(
          RVISA::Quadrant::QUADRANT1, Token("c.jal"), 0b001)));
    }

    instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCBType(
        RVISA::Quadrant::QUADRANT1, Token("c.beqz"), 0b110, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCBType(
        RVISA::Quadrant::QUADRANT1, Token("c.bnez"), 0b111, isa)));

    instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCIWType(
        RVISA::Quadrant::QUADRANT0, Token("c.addi4spn"), 0b000, isa)));

    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCB2Type(RVISA::Quadrant::QUADRANT1, Token("c.srli"), 0b00,
                            0b100, Imm::Repr::Unsigned, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCB2Type(RVISA::Quadrant::QUADRANT1, Token("c.srai"), 0b01,
                            0b100, Imm::Repr::Unsigned, isa)));

    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCB2Type(RVISA::Quadrant::QUADRANT1, Token("c.andi"), 0b10,
                            0b100, Imm::Repr::Signed, isa)));

    instructions.push_back(std::shared_ptr<Instruction>(
        new RVCInstrCRType(RVISA::Quadrant::QUADRANT2, Token("c.mv"), 0b1000,
                           isa))); // FIXME disassemble erro with c.jr ?
    instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCRType(
        RVISA::Quadrant::QUADRANT2, Token("c.add"), 0b1001, isa)));

    instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCR2Type(
        RVISA::Quadrant::QUADRANT2, Token("c.jr"), 0b1000, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(new RVCInstrCR2Type(
        RVISA::Quadrant::QUADRANT2, Token("c.jalr"), 0b1001, isa)));

    // instructions.push_back(CREBREAKType(0b10, Token("c.ebreak"), 0b1001));
    // //FIXME Duplicated terminate called after throwing an instance of
    // 'std::runtime_error' what():  Instruction cannot be decoded; aliases with
    // other instruction (Identical to other instruction) c.ebreak is equal to
    // c.jalr
  }
};

} // namespace Assembler
} // namespace Ripes
