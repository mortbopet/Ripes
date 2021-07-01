#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "instruction.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

/// Register specialization for RV-C to handle the compressed register notation. 3 bits are used to represent registers
/// x8-x15, with register x8=0b000, x15=0b111.
template <typename Reg_T>
struct RVCReg : public Reg<Reg_T> {
    using Reg<Reg_T>::Reg;
    std::optional<Error> apply(const TokenizedSrcLine& line, Instr_T& instruction,
                               FieldLinkRequest<Reg_T>&) const override {
        bool success;
        const QString& regToken = line.tokens[this->tokenIndex];
        unsigned reg = this->m_isa->regNumber(regToken, success);
        if (!success)
            return Error(line.sourceLine, "Unknown register '" + regToken + "'");
        if (!(8 <= reg && reg <= 15))
            return Error(line.sourceLine, "Only registers x8-x15 are allowed for this instruction");
        reg -= 8;
        instruction |= this->m_range.apply(reg);
        return std::nullopt;
    }

    std::optional<Error> decode(const Instr_T instruction, const Reg_T /*address*/, const ReverseSymbolMap&,
                                LineTokens& line) const override {
        const unsigned regNumber = this->m_range.decode(instruction) + 8;
        const Token registerName = this->m_isa->regName(regNumber);
        if (registerName.isEmpty()) {
            return Error(0, "Unknown register number '" + QString::number(regNumber) + "'");
        }
        line.push_back(registerName);
        return std::nullopt;
    }
};

#define CAType(name, funct2, funct6)                                                                               \
    std::shared_ptr<_Instruction>(                                                                                 \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(0b01, 0, 1), OpPart(funct2, 5, 6), OpPart(funct6, 10, 15)}), \
                         {std::make_shared<RVCReg<Reg__T>>(isa, 2, 2, 4, "rs2'"),                                  \
                          std::make_shared<RVCReg<Reg__T>>(isa, 1, 7, 9, "rd'/rs1'")}))

#define CIType(opcode, name, funct3, imm)                                                      \
    std::shared_ptr<_Instruction>(                                                             \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct3, 13, 15)}), \
                         {std::make_shared<_Reg>(isa, 1, 7, 11, "rd/rs1"), imm}))

#define CINOPType(opcode, name) \
    std::shared_ptr<_Instruction>(new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(0, 2, 15)}), {}))

#define CSSType(opcode, name, funct3, imm)                                                     \
    std::shared_ptr<_Instruction>(                                                             \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct3, 13, 15)}), \
                         {std::make_shared<_Reg>(isa, 1, 2, 6, "rs2"), imm}))

#define CLType(opcode, name, funct3, imm)                                                      \
    std::shared_ptr<_Instruction>(                                                             \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct3, 13, 15)}), \
                         {std::make_shared<RVCReg<Reg__T>>(isa, 1, 2, 4, "rd'"),               \
                          std::make_shared<RVCReg<Reg__T>>(isa, 2, 7, 9, "rs1'"), imm}))

#define CSType(opcode, name, funct3)                                          \
    std::shared_ptr<_Instruction>(new _Instruction(                           \
        Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct3, 13, 15)}), \
        {std::make_shared<RVCReg<Reg__T>>(isa, 1, 2, 4, "rs2'"),              \
         std::make_shared<RVCReg<Reg__T>>(isa, 2, 7, 9, "rs1'"),              \
         std::make_shared<_Imm>(3, 7, _Imm::Repr::Unsigned,                   \
                                std::vector{ImmPart(6, 5, 5), ImmPart(3, 10, 12), ImmPart(2, 6, 6)})}))

#define CJType(opcode, name, funct3)                                                                                  \
    std::shared_ptr<_Instruction>(                                                                                    \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct3, 13, 15)}),                        \
                         {std::make_shared<_Imm>(                                                                     \
                             1, 12, _Imm::Repr::Signed,                                                               \
                             std::vector{ImmPart(11, 12, 12), ImmPart(10, 8, 8), ImmPart(8, 9, 10), ImmPart(7, 6, 6), \
                                         ImmPart(6, 7, 7), ImmPart(5, 2, 2), ImmPart(4, 11, 11), ImmPart(1, 3, 5)})}))

#define CRType(opcode, name, funct4)                                                           \
    std::shared_ptr<_Instruction>(                                                             \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct4, 12, 15)}), \
                         {std::make_shared<_Reg>(isa, 1, 7, 11, "rs1"), std::make_shared<_Reg>(isa, 2, 2, 6, "rs2")}))

#define CR2Type(opcode, name, funct4)                                                                           \
    std::shared_ptr<_Instruction>(                                                                              \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(0, 2, 6), OpPart(funct4, 12, 15)}), \
                         {std::make_shared<_Reg>(isa, 1, 7, 11, "rs1")}))

#define CREBREAKType(opcode, name, funct4) \
    std::shared_ptr<_Instruction>(         \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(0, 2, 11), OpPart(funct4, 12, 15)}), {}))

#define CBType(opcode, name, funct3)                                                                                 \
    std::shared_ptr<_Instruction>(                                                                                   \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct3, 13, 15)}),                       \
                         {std::make_shared<RVCReg<Reg__T>>(isa, 1, 7, 9, "rs1'"),                                    \
                          std::make_shared<_Imm>(2, 9, _Imm::Repr::Signed,                                           \
                                                 std::vector{ImmPart(8, 12, 12), ImmPart(6, 5, 6), ImmPart(5, 2, 2), \
                                                             ImmPart(3, 10, 11), ImmPart(1, 3, 4)})}))

#define CB2Type(opcode, name, funct3, funct4, imm)                                                                     \
    std::shared_ptr<_Instruction>(                                                                                     \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct4, 10, 11), OpPart(funct3, 13, 15)}), \
                         {std::make_shared<RVCReg<Reg__T>>(isa, 1, 7, 9, "rs1'"), imm}))

#define CIWType(opcode, name, funct3)                                         \
    std::shared_ptr<_Instruction>(new _Instruction(                           \
        Opcode<Reg__T>(name, {OpPart(opcode, 0, 1), OpPart(funct3, 13, 15)}), \
        {std::make_shared<RVCReg<Reg__T>>(isa, 1, 2, 4, "rd'"),               \
         std::make_shared<_Imm>(                                              \
             2, 10, _Imm::Repr::Unsigned,                                     \
             std::vector{ImmPart(6, 7, 10), ImmPart(4, 11, 12), ImmPart(3, 5, 5), ImmPart(2, 6, 6)})}))

/**
 * Extension enabler.
 * Calling an extension enabler will register the appropriate assemblers and pseudo-op expander functors with
 * the assembler.
 * The extension enablers are templated to allow for sharing implementations between 32- and 64-bit variants.
 */
template <typename Reg__T>
struct RV_C {
    AssemblerTypes(Reg__T);
    static void enable(const ISAInfoBase* isa, _InstrVec& instructions, _PseudoInstrVec& /*pseudoInstructions*/) {
        // Pseudo-op functors

        // Assembler functors
        instructions.push_back(CAType(Token("c.sub"), 0b00, 0b100011));
        instructions.push_back(CAType(Token("c.xor"), 0b01, 0b100011));
        instructions.push_back(CAType(Token("c.or"), 0b10, 0b100011));
        instructions.push_back(CAType(Token("c.and"), 0b11, 0b100011));
        instructions.push_back(CAType(Token("c.subw"), 0b00, 0b100111));
        instructions.push_back(CAType(Token("c.addw"), 0b01, 0b100111));

        instructions.push_back(
            CIType(0b10, Token("c.lwsp"), 0b010,
                   std::make_shared<_Imm>(2, 8, _Imm::Repr::Unsigned,
                                          std::vector{ImmPart(6, 2, 3), ImmPart(5, 12, 12), ImmPart(2, 4, 6)})));

        if (isa->isaID() == ISA::RV32I) {
            instructions.push_back(
                CIType(0b10, Token("c.flwsp"), 0b011,
                       std::make_shared<_Imm>(2, 8, _Imm::Repr::Unsigned,
                                              std::vector{ImmPart(6, 2, 3), ImmPart(5, 12, 12), ImmPart(2, 4, 6)})));
        } else  // RV64 RV128
        {
            instructions.push_back(
                CIType(0b10, Token("c.ldsp"), 0b011,
                       std::make_shared<_Imm>(2, 9, _Imm::Repr::Unsigned,
                                              std::vector{ImmPart(6, 2, 4), ImmPart(5, 12, 12), ImmPart(3, 5, 6)})));
            instructions.push_back(CIType(
                0b01, Token("c.addiw"), 0b001,
                std::make_shared<_Imm>(2, 6, _Imm::Repr::Signed, std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));
        }

        // instructions.push_back(CIType(0b10, Token("c.lqsp"), 0b001));//RV128
        instructions.push_back(
            CIType(0b10, Token("c.fldsp"), 0b001,
                   std::make_shared<_Imm>(2, 9, _Imm::Repr::Unsigned,
                                          std::vector{ImmPart(6, 2, 4), ImmPart(5, 12, 12), ImmPart(3, 5, 6)})));
        instructions.push_back(CIType(
            0b10, Token("c.slli"), 0b000,
            std::make_shared<_Imm>(2, 6, _Imm::Repr::Unsigned, std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));

        instructions.push_back(CIType(
            0b01, Token("c.li"), 0b010,
            std::make_shared<_Imm>(2, 6, _Imm::Repr::Signed, std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));

        auto cLuiInstr = CIType(
            0b01, Token("c.lui"), 0b011,
            std::make_shared<_Imm>(2, 18, _Imm::Repr::Signed, std::vector{ImmPart(17, 12, 12), ImmPart(12, 2, 6)}));
        cLuiInstr->addExtraMatchCond([](Instr_T instr) {
            unsigned rd = (instr >> 7) & 0b11111;
            return rd != 0 && rd != 2;
        });

        instructions.push_back(cLuiInstr);

        auto cAddi16spInstr = std::shared_ptr<_Instruction>(new _Instruction(
            Opcode<Reg__T>(Token("c.addi16sp"), {OpPart(0b01, 0, 1), OpPart(0b011, 13, 15), OpPart(2, 7, 11)}),
            {std::make_shared<_Imm>(1, 10, _Imm::Repr::Signed,
                                    std::vector{ImmPart(9, 12, 12), ImmPart(7, 3, 4), ImmPart(6, 5, 5),
                                                ImmPart(5, 2, 2), ImmPart(4, 6, 6)})}));
        cAddi16spInstr->addExtraMatchCond([](Instr_T instr) {
            unsigned rd = (instr >> 7) & 0b11111;
            return rd == 2;
        });
        instructions.push_back(cAddi16spInstr);

        instructions.push_back(CIType(
            0b01, Token("c.addi"), 0b000,
            std::make_shared<_Imm>(2, 6, _Imm::Repr::Signed, std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));
        instructions.push_back(CINOPType(0b01, Token("c.nop")));

        instructions.push_back(CSSType(
            0b10, Token("c.swsp"), 0b110,
            std::make_shared<_Imm>(2, 8, _Imm::Repr::Unsigned, std::vector{ImmPart(6, 7, 8), ImmPart(2, 9, 12)})));
        if (isa->isaID() == ISA::RV32I) {
            instructions.push_back(CSSType(
                0b10, Token("c.fswsp"), 0b111,
                std::make_shared<_Imm>(2, 8, _Imm::Repr::Unsigned, std::vector{ImmPart(6, 7, 8), ImmPart(2, 9, 12)})));
        } else {
            instructions.push_back(CSSType(
                0b10, Token("c.sdsp"), 0b111,
                std::make_shared<_Imm>(2, 9, _Imm::Repr::Unsigned, std::vector{ImmPart(6, 7, 9), ImmPart(3, 10, 12)})));
        }
        instructions.push_back(CSSType(
            0b10, Token("c.fsdsp"), 0b101,
            std::make_shared<_Imm>(2, 9, _Imm::Repr::Unsigned, std::vector{ImmPart(6, 7, 9), ImmPart(3, 10, 12)})));
        // instructions.push_back(CSSType(0b10, Token("c.sqsp"), 0b101));//RV128

        instructions.push_back(
            CLType(0b00, Token("c.lw"), 0b010,
                   std::make_shared<_Imm>(3, 7, _Imm::Repr::Signed,
                                          std::vector{ImmPart(6, 5, 5), ImmPart(3, 10, 12), ImmPart(2, 6, 6)})));
        if (isa->isaID() == ISA::RV32I) {
            instructions.push_back(
                CLType(0b00, Token("c.flw"), 0b011,
                       std::make_shared<_Imm>(3, 7, _Imm::Repr::Signed,
                                              std::vector{ImmPart(6, 5, 5), ImmPart(3, 10, 12), ImmPart(2, 6, 6)})));
        } else {
            instructions.push_back(CLType(
                0b00, Token("c.ld"), 0b011,
                std::make_shared<_Imm>(3, 8, _Imm::Repr::Signed, std::vector{ImmPart(6, 5, 6), ImmPart(3, 10, 12)})));
        }
        // instructions.push_back(CLType(0b00, Token("c.lq"), 0b001));//RV128
        instructions.push_back(CLType(
            0b00, Token("c.fld"), 0b001,
            std::make_shared<_Imm>(3, 8, _Imm::Repr::Signed, std::vector{ImmPart(6, 5, 6), ImmPart(3, 10, 12)})));

        instructions.push_back(CSType(0b00, Token("c.sw"), 0b110));
        if (isa->isaID() == ISA::RV32I) {
            instructions.push_back(CSType(0b00, Token("c.fsw"), 0b111));
        } else {
            instructions.push_back(CSType(0b00, Token("c.sd"), 0b111));
        }
        // instructions.push_back(CSType(0b00, Token("c.sq"), 0b101));//RV128
        instructions.push_back(CSType(0b00, Token("c.fsd"), 0b101));

        instructions.push_back(CJType(0b01, Token("c.j"), 0b101));
        if (isa->isaID() == ISA::RV32I) {
            instructions.push_back(CJType(0b01, Token("c.jal"), 0b001));
        }

        instructions.push_back(CBType(0b01, Token("c.beqz"), 0b110));
        instructions.push_back(CBType(0b01, Token("c.bnez"), 0b111));

        instructions.push_back(CIWType(0b00, Token("c.addi4spn"), 0b000));

        instructions.push_back(CB2Type(
            0b01, Token("c.srli"), 0b100, 0b00,
            std::make_shared<_Imm>(2, 6, _Imm::Repr::Unsigned, std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));
        instructions.push_back(CB2Type(
            0b01, Token("c.srai"), 0b100, 0b01,
            std::make_shared<_Imm>(2, 6, _Imm::Repr::Unsigned, std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));

        instructions.push_back(CB2Type(
            0b01, Token("c.andi"), 0b100, 0b10,
            std::make_shared<_Imm>(2, 6, _Imm::Repr::Signed, std::vector{ImmPart(5, 12, 12), ImmPart(0, 2, 6)})));

        instructions.push_back(CRType(0b10, Token("c.mv"), 0b1000));  // FIXME disassemble erro with c.jr ?
        instructions.push_back(CRType(0b10, Token("c.add"), 0b1001));

        instructions.push_back(CR2Type(0b10, Token("c.jr"), 0b1000));
        instructions.push_back(CR2Type(0b10, Token("c.jalr"), 0b1001));

        // instructions.push_back(CREBREAKType(0b10, Token("c.ebreak"), 0b1001)); //FIXME Duplicated
        // terminate called after throwing an instance of 'std::runtime_error'
        // what():  Instruction cannot be decoded; aliases with other instruction (Identical to other instruction)
        // c.ebreak is equal to c.jalr
    }
};

}  // namespace Assembler
}  // namespace Ripes
