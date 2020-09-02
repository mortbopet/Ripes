#include "rv32i_assembler.h"

#include <QByteArray>
#include <algorithm>

namespace Ripes {
namespace AssemblerTmp {

RV32I_Assembler::RV32I_Assembler(const std::set<Extensions>& extensions)
    : AssemblerBase<ISAInfo<ISA::RV32IM>>(initInstructions(extensions)) {}

std::pair<RV32I_Assembler::RVInstrVec, RV32I_Assembler::RVPseudoInstrVec>
RV32I_Assembler::initInstructions(const std::set<Extensions>& extensions) const {
    RVInstrVec instructions;
    RVPseudoInstrVec pseudoInstructions;

    enableExtI(instructions, pseudoInstructions);
    for (const auto& extension : extensions) {
        switch (extension) {
            case Extensions::M:
                enableExtM(instructions, pseudoInstructions);
                break;
            case Extensions::F:
                enableExtF(instructions, pseudoInstructions);
                break;
            default:
                assert(false && "Unhandled ISA extension");
        }
    }
    return {instructions, pseudoInstructions};
}

std::variant<AssemblerTmp::Error, std::optional<std::vector<AssemblerTmp::LineTokens>>>
RV32I_Assembler::expandPseudoOp(const SourceLine& line) const {
    if (line.tokens.empty()) {
        return {};
    }
    const auto& opcode = line.tokens.at(0);
    if (m_pseudoOpExpanders.count(opcode) == 0) {
        return {};
    }
    return m_pseudoOpExpanders.at(opcode)(line);
}

std::variant<AssemblerTmp::Error, QByteArray>
RV32I_Assembler::assembleInstruction(const SourceLine& line, const AssemblerTmp::SymbolMap& symbols) const {
    if (line.tokens.empty()) {
        return {QByteArray()};
    }
    const auto& opcode = line.tokens.at(0);
    if (m_assemblers.count(opcode)) {
        return {Error(line.source_line, "Unknown opcode '" + opcode + "'")};
    }
    return {m_assemblers.at(opcode)(line, symbols)};
}

void RV32I_Assembler::addPseudoOpExpanderFunctor(const QString& opcode, PseudoOpFunctor functor) {
    if (m_pseudoOpExpanders.count(opcode)) {
        assert(false && "Pseudo-op expander already registerred for opcode");
    }
    m_pseudoOpExpanders[opcode] = functor;
}

void RV32I_Assembler::addAssemblerFunctor(const QString& opcode, AssemblerFunctor functor) {
    if (m_assemblers.count(opcode)) {
        assert(false && "Assembler already registerred for opcode");
    }
    m_assemblers[opcode] = functor;
}
#define BType(name, funct3)                                                                              \
    std::shared_ptr<Instruction<ISAInfo<ISA::RV32IM>>>(new Instruction<ISAInfo<ISA::RV32IM>>(            \
        Opcode(name, {OpPart(0b1100011, 0, 6), OpPart(funct3, 12, 14)}),                                 \
        {std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(1, 15, 19),                                         \
         std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(2, 20, 24),                                         \
         std::make_shared<Imm>(                                                                          \
             3, 13, Imm::Repr::Signed,                                                                   \
             std::vector{ImmPart(12, 31, 31), ImmPart(11, 7, 7), ImmPart(5, 25, 30), ImmPart(1, 8, 11)}, \
             Imm::SymbolType::Relative)}))

#define IType(name, funct3)                                                                   \
    std::shared_ptr<Instruction<ISAInfo<ISA::RV32IM>>>(new Instruction<ISAInfo<ISA::RV32IM>>( \
        Opcode(name, {OpPart(0b0010011, 0, 6), OpPart(funct3, 12, 14)}),                      \
        {std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(1, 7, 11),                               \
         std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(2, 15, 19),                              \
         std::make_shared<Imm>(3, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define LoadType(name, funct3)                                                                \
    std::shared_ptr<Instruction<ISAInfo<ISA::RV32IM>>>(new Instruction<ISAInfo<ISA::RV32IM>>( \
        Opcode(name, {OpPart(0b0000011, 0, 6), OpPart(funct3, 12, 14)}),                      \
        {std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(1, 7, 11),                               \
         std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(3, 15, 19),                              \
         std::make_shared<Imm>(2, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define IShiftType(name, funct3, funct7)                                                         \
    std::shared_ptr<Instruction<ISAInfo<ISA::RV32IM>>>(new Instruction<ISAInfo<ISA::RV32IM>>(    \
        Opcode(name, {OpPart(0b0010011, 0, 6), OpPart(funct3, 12, 14), OpPart(funct7, 25, 31)}), \
        {std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(1, 7, 11),                                  \
         std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(2, 15, 19),                                 \
         std::make_shared<Imm>(3, 5, Imm::Repr::Unsigned, std::vector{ImmPart(0, 20, 24)})}))

#define RType(name, funct3, funct7)                                                              \
    std::shared_ptr<Instruction<ISAInfo<ISA::RV32IM>>>(new Instruction<ISAInfo<ISA::RV32IM>>(    \
        Opcode(name, {OpPart(0b0110011, 0, 6), OpPart(funct3, 12, 14), OpPart(funct7, 25, 31)}), \
        {std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(1, 7, 11),                                  \
         std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(2, 15, 19),                                 \
         std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(3, 20, 24)}))

#define SType(name, funct3)                                                                                   \
    std::shared_ptr<Instruction<ISAInfo<ISA::RV32IM>>>(new Instruction<ISAInfo<ISA::RV32IM>>(                 \
        Opcode(name, {OpPart(0b0100011, 0, 6), OpPart(funct3, 12, 14)}),                                      \
        {std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(3, 15, 19),                                              \
         std::make_shared<Imm>(2, 12, Imm::Repr::Signed, std::vector{ImmPart(5, 25, 31), ImmPart(0, 7, 11)}), \
         std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(1, 20, 24)}))

#define UType(name, opcode)                                                                   \
    std::shared_ptr<Instruction<ISAInfo<ISA::RV32IM>>>(new Instruction<ISAInfo<ISA::RV32IM>>( \
        Opcode(name, {OpPart(opcode, 0, 6)}),                                                 \
        {std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(1, 7, 11),                               \
         std::make_shared<Imm>(2, 32, Imm::Repr::Hex, std::vector{ImmPart(12, 12, 31)})}))

#define JType(name, opcode)                                                                                  \
    std::shared_ptr<Instruction<ISAInfo<ISA::RV32IM>>>(new Instruction<ISAInfo<ISA::RV32IM>>(                \
        Opcode(name, {OpPart(opcode, 0, 6)}),                                                                \
        {std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(1, 7, 11),                                              \
         std::make_shared<Imm>(                                                                              \
             2, 21, Imm::Repr::Hex,                                                                          \
             std::vector{ImmPart(20, 31, 31), ImmPart(12, 12, 19), ImmPart(11, 20, 20), ImmPart(1, 21, 30)}, \
             Imm::SymbolType::Relative)}))

#define JALRType(name)                                                                        \
    std::shared_ptr<Instruction<ISAInfo<ISA::RV32IM>>>(new Instruction<ISAInfo<ISA::RV32IM>>( \
        Opcode(name, {OpPart(0b1100111, 0, 6), OpPart(0b000, 12, 14)}),                       \
        {std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(1, 7, 11),                               \
         std::make_shared<Reg<ISAInfo<ISA::RV32IM>>>(2, 15, 19),                              \
         std::make_shared<Imm>(3, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

void RV32I_Assembler::enableExtI(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const {
    // Pseudo-op functors
    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("nop", {}, [](const RVPseudoInstr& _this, const AssemblerTmp::SourceLine& line) {
            return std::vector<AssemblerTmp::LineTokens>{QString("addi x0 x0 0").split(' ')};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("mv", {}, [](const RVPseudoInstr& _this, const AssemblerTmp::SourceLine& line) {
            return std::vector<AssemblerTmp::LineTokens>{QString("addi x0 x0 0").split(' ')};
        })));

    /*        m_expander = [this](const PseudoInstruction& _this, const AssemblerTmp::SourceLine& line) {
            uint32_t instruction = 0;
            m_opcode.apply(line, instruction);
            for (const auto& field : m_fields)
                field->apply(line, instruction);
            return instruction;
        };*/

    // Assembler functors
    instructions.push_back(UType("lui", 0b0110111));
    instructions.push_back(UType("auipc", 0b0010111));

    instructions.push_back(JType("jal", 0b1101111));

    instructions.push_back(JALRType("jalr"));

    instructions.push_back(LoadType("lb", 0b000));
    instructions.push_back(LoadType("lh", 0b001));
    instructions.push_back(LoadType("lw", 0b010));
    instructions.push_back(LoadType("lbu", 0b100));
    instructions.push_back(LoadType("lhu", 0b101));

    instructions.push_back(SType("sb", 0b000));
    instructions.push_back(SType("sh", 0b001));
    instructions.push_back(SType("sw", 0b010));

    instructions.push_back(IType("addi", 0b000));
    instructions.push_back(IType("slti", 0b010));
    instructions.push_back(IType("sltiu", 0b011));
    instructions.push_back(IType("xori", 0b100));
    instructions.push_back(IType("ori", 0b110));
    instructions.push_back(IType("andi", 0b111));

    instructions.push_back(IShiftType("slli", 0b001, 0b0000000));
    instructions.push_back(IShiftType("srli", 0b101, 0b0000000));
    instructions.push_back(IShiftType("srai", 0b101, 0b0100000));

    instructions.push_back(RType("add", 0b000, 0b0000000));
    instructions.push_back(RType("sub", 0b000, 0b0100000));
    instructions.push_back(RType("sll", 0b001, 0b0000000));
    instructions.push_back(RType("slt", 0b010, 0b0000000));
    instructions.push_back(RType("sltu", 0b011, 0b0000000));
    instructions.push_back(RType("xor", 0b100, 0b0000000));
    instructions.push_back(RType("srl", 0b101, 0b0000000));
    instructions.push_back(RType("sra", 0b101, 0b0100000));
    instructions.push_back(RType("or", 0b110, 0b0000000));
    instructions.push_back(RType("and", 0b111, 0b0000000));

    instructions.push_back(BType("beq", 0b000));
    instructions.push_back(BType("bne", 0b001));
    instructions.push_back(BType("blt", 0b100));
    instructions.push_back(BType("bge", 0b101));
    instructions.push_back(BType("bltu", 0b110));
    instructions.push_back(BType("bgeu", 0b111));
}

void RV32I_Assembler::enableExtM(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const {
    // Pseudo-op functors

    // Assembler functors
}
void RV32I_Assembler::enableExtF(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const {
    // Pseudo-op functors

    // Assembler functors
}

}  // namespace AssemblerTmp
}  // namespace Ripes
