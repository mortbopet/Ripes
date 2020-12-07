#include "rv32i_assembler.h"
#include "gnudirectives.h"

#include <QByteArray>
#include <algorithm>

namespace Ripes {
namespace AssemblerTmp {

RV32I_Assembler::RV32I_Assembler(const std::set<Extensions> extensions) : Assembler<ISAInfo<ISA::RV32IM>>() {
    auto [instrs, pseudos, directives] = initInstructions(extensions);
    initialize(instrs, pseudos, directives);

    // Initialize segment pointers
    setSegmentBase(".text", 0x0);
    setSegmentBase(".data", 0x10000000);
    setSegmentBase(".bss", 0x11000000);
}

std::tuple<RV32I_Assembler::RVInstrVec, RV32I_Assembler::RVPseudoInstrVec, DirectiveVec>
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
    return {instructions, pseudoInstructions, gnuDirectives()};
}

#define BType(name, funct3)                                                                                         \
    std::shared_ptr<RVInstr>(new RVInstr(Opcode(name, {OpPart(0b1100011, 0, 6), OpPart(funct3, 12, 14)}),           \
                                         {std::make_shared<RVReg>(1, 15, 19), std::make_shared<RVReg>(2, 20, 24),   \
                                          std::make_shared<Imm>(3, 13, Imm::Repr::Signed,                           \
                                                                std::vector{ImmPart(12, 31, 31), ImmPart(11, 7, 7), \
                                                                            ImmPart(5, 25, 30), ImmPart(1, 8, 11)}, \
                                                                Imm::SymbolType::Relative)}))

#define IType(name, funct3)                                                                 \
    std::shared_ptr<RVInstr>(                                                               \
        new RVInstr(Opcode(name, {OpPart(0b0010011, 0, 6), OpPart(funct3, 12, 14)}),        \
                    {std::make_shared<RVReg>(1, 7, 11), std::make_shared<RVReg>(2, 15, 19), \
                     std::make_shared<Imm>(3, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define LoadType(name, funct3)                                                              \
    std::shared_ptr<RVInstr>(                                                               \
        new RVInstr(Opcode(name, {OpPart(0b0000011, 0, 6), OpPart(funct3, 12, 14)}),        \
                    {std::make_shared<RVReg>(1, 7, 11), std::make_shared<RVReg>(3, 15, 19), \
                     std::make_shared<Imm>(2, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define IShiftType(name, funct3, funct7)                                                                     \
    std::shared_ptr<RVInstr>(                                                                                \
        new RVInstr(Opcode(name, {OpPart(0b0010011, 0, 6), OpPart(funct3, 12, 14), OpPart(funct7, 25, 31)}), \
                    {std::make_shared<RVReg>(1, 7, 11), std::make_shared<RVReg>(2, 15, 19),                  \
                     std::make_shared<Imm>(3, 5, Imm::Repr::Unsigned, std::vector{ImmPart(0, 20, 24)})}))

#define RType(name, funct3, funct7)                                                              \
    std::shared_ptr<RVInstr>(new RVInstr(                                                        \
        Opcode(name, {OpPart(0b0110011, 0, 6), OpPart(funct3, 12, 14), OpPart(funct7, 25, 31)}), \
        {std::make_shared<RVReg>(1, 7, 11), std::make_shared<RVReg>(2, 15, 19), std::make_shared<RVReg>(3, 20, 24)}))

#define SType(name, funct3)                                                                                   \
    std::shared_ptr<RVInstr>(new RVInstr(                                                                     \
        Opcode(name, {OpPart(0b0100011, 0, 6), OpPart(funct3, 12, 14)}),                                      \
        {std::make_shared<RVReg>(3, 15, 19),                                                                  \
         std::make_shared<Imm>(2, 12, Imm::Repr::Signed, std::vector{ImmPart(5, 25, 31), ImmPart(0, 7, 11)}), \
         std::make_shared<RVReg>(1, 20, 24)}))

#define UType(name, opcode)                               \
    std::shared_ptr<RVInstr>(                             \
        new RVInstr(Opcode(name, {OpPart(opcode, 0, 6)}), \
                    {std::make_shared<RVReg>(1, 7, 11),   \
                     std::make_shared<Imm>(2, 32, Imm::Repr::Hex, std::vector{ImmPart(0, 12, 31)})}))

#define JType(name, opcode)                                                                                           \
    std::shared_ptr<RVInstr>(new RVInstr(Opcode(name, {OpPart(opcode, 0, 6)}),                                        \
                                         {std::make_shared<RVReg>(1, 7, 11),                                          \
                                          std::make_shared<Imm>(2, 21, Imm::Repr::Hex,                                \
                                                                std::vector{ImmPart(20, 31, 31), ImmPart(12, 12, 19), \
                                                                            ImmPart(11, 20, 20), ImmPart(1, 21, 30)}, \
                                                                Imm::SymbolType::Relative)}))

#define JALRType(name)                                                                      \
    std::shared_ptr<RVInstr>(                                                               \
        new RVInstr(Opcode(name, {OpPart(0b1100111, 0, 6), OpPart(0b000, 12, 14)}),         \
                    {std::make_shared<RVReg>(1, 7, 11), std::make_shared<RVReg>(2, 15, 19), \
                     std::make_shared<Imm>(3, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define RegTok RVPseudoInstr::reg()
#define ImmTok RVPseudoInstr::imm()
#define CreatePseudoInstruction
#define PseudoExpandFunc(line) [](const RVPseudoInstr&, const AssemblerTmp::TokenizedSrcLine& line)

#define PseudoLoad(name)                                                                                \
    std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(                                                   \
        name, {RegTok, ImmTok}, PseudoExpandFunc(line) {                                                \
            LineTokensVec v;                                                                            \
            v.push_back(QStringList() << "auipc" << line.tokens.at(1) << line.tokens.at(2));            \
            v.push_back(QStringList() << name << line.tokens.at(1)                                      \
                                      << QString("((%1&0xfff)-(__address__-4))").arg(line.tokens.at(2)) \
                                      << line.tokens.at(1));                                            \
            return v;                                                                                   \
        }))

// The sw is a pseudo-op if a symbol is given as the immediate token. Thus, if we detect that
// a number has been provided, then abort the pseudo-op handling.
#define PseudoStore(name)                                                                               \
    std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(                                                   \
        name, {RegTok, ImmTok, RegTok}, PseudoExpandFunc(line) {                                        \
            bool canConvert;                                                                            \
            getImmediate(line.tokens.at(2), canConvert);                                                \
            if (canConvert) {                                                                           \
                return PseudoExpandRes(Error(0, "Unused; will fallback to non-pseudo op sw"));          \
            }                                                                                           \
            LineTokensVec v;                                                                            \
            v.push_back(QStringList() << "auipc" << line.tokens.at(3) << line.tokens.at(2));            \
            v.push_back(QStringList() << name << line.tokens.at(1)                                      \
                                      << QString("((%1&0xfff)-(__address__-4))").arg(line.tokens.at(2)) \
                                      << line.tokens.at(3));                                            \
            return PseudoExpandRes(v);                                                                  \
        }))

void RV32I_Assembler::enableExtI(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const {
    // Pseudo-op functors
    pseudoInstructions.push_back(PseudoLoad("lb"));
    pseudoInstructions.push_back(PseudoLoad("lh"));
    pseudoInstructions.push_back(PseudoLoad("lw"));
    pseudoInstructions.push_back(PseudoStore("sb"));
    pseudoInstructions.push_back(PseudoStore("sh"));
    pseudoInstructions.push_back(PseudoStore("sw"));

    // clang-format off
    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "la", {RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList() << "auipc" << line.tokens.at(1) << line.tokens.at(2),
                                 QStringList() << "addi" << line.tokens.at(1) << line.tokens.at(1) << line.tokens.at(2)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "call", {RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << "auipc" << "x6" << line.tokens.at(1),
                                 LineTokens() << "jalr" << "x1" << "x6" << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "tail", {RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList() << "auipc" << "x6" << line.tokens.at(1),
                                 QStringList() << "jalr" << "x0" << "x6" << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "j", {ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList() << "jal" << "x0" << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "jr", {RegTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList() << "jalr" << "x0" << line.tokens.at(1) << "0"};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "jalr", {RegTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList() << "jalr" << "x1" << line.tokens.at(1) << "0"};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "ret", {}, [](const RVPseudoInstr&, const AssemblerTmp::TokenizedSrcLine&) {
            return LineTokensVec{QStringList() << "jalr" << "x0" << "x1" << "0"};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "jal", {ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList() << "jal" << "x1" << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "li", {RegTok, ImmTok},
        PseudoExpandFunc(line) {
            LineTokensVec v;
            // Determine whether an ADDI or LUI instruction is sufficient, or if both LUI and ADDI is needed, by
            // analysing the immediate size
            bool canConvert;
            int immediate = getImmediate(line.tokens.at(2), canConvert);

            if(!canConvert) {
                return PseudoExpandRes{Error(line.sourceLine, QString("Invalid immediate '%1'").arg(line.tokens.at(2)))};
            }

            // Generate offset required for discerning between positive and negative immediates
            if (isInt<12>(immediate)) {
                // immediate can be represented by 12 bits, ADDI is sufficient
                v.push_back(QStringList() << "addi" << line.tokens.at(1) << "x0" << QString::number(immediate));
            } else {
                const int lower12Signed = signextend<int32_t, 12>(immediate & 0xFFF);
                int signOffset = lower12Signed < 0 ? 1 : 0;
                v.push_back(QStringList() << "lui" << line.tokens.at(1)
                                          << QString::number((static_cast<uint32_t>(immediate) >> 12) + signOffset));
                if ((immediate & 0xFFF) != 0) {
                    v.push_back(QStringList()
                                << "addi" << line.tokens.at(1) << line.tokens.at(1) << QString::number(lower12Signed));
                }
            }
            return PseudoExpandRes{v};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("nop", {}, [](const RVPseudoInstr&, const AssemblerTmp::TokenizedSrcLine&) {
            return LineTokensVec{QString("addi x0 x0 0").split(' ')};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(new RVPseudoInstr(
        "mv", {RegTok, RegTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"addi", line.tokens.at(1), line.tokens.at(2), "0"}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("not", {RegTok, RegTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"xori", line.tokens.at(1), line.tokens.at(2), "-1"}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("neg", {RegTok, RegTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"sub", line.tokens.at(1), "x0", line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("seqz", {RegTok, RegTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"sltiu", line.tokens.at(1), line.tokens.at(2), "1"}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("snez", {RegTok, RegTok}, PseudoExpandFunc(line) {
            return LineTokensVec{ QStringList{"sltu", line.tokens.at(1), "x0", line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("sltz", {RegTok, RegTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"slt", line.tokens.at(1), line.tokens.at(2), "x0"}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("sgtz", {RegTok, RegTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"slt", line.tokens.at(1), "x0", line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("beqz", {RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"beq", line.tokens.at(1), "x0", line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("bnez", {RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"bne", line.tokens.at(1), "x0", line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("blez", {RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"bge", "x0", line.tokens.at(1), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("bgez", {RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"bge", line.tokens.at(1), "x0", line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("bltz", {RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"blt", line.tokens.at(1), "x0", line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("bgtz", {RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"blt", "x0", line.tokens.at(1), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("bgt", {RegTok, RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"blt", line.tokens.at(2), line.tokens.at(1), line.tokens.at(3)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("ble", {RegTok, RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"bge", line.tokens.at(2), line.tokens.at(1), line.tokens.at(3)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("bgtu", {RegTok, RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"bltu", line.tokens.at(2), line.tokens.at(2), line.tokens.at(3)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<RVPseudoInstr>(
        new RVPseudoInstr("bleu", {RegTok, RegTok, ImmTok}, PseudoExpandFunc(line) {
            return LineTokensVec{QStringList{"bgeu", line.tokens.at(2), line.tokens.at(2), line.tokens.at(3)}};
        })));
    // clang-format on

    // Assembler functors

    instructions.push_back(
        std::shared_ptr<RVInstr>(new RVInstr(Opcode("ecall", {OpPart(0b1110011, 0, 6), OpPart(0, 7, 31)}), {})));

    instructions.push_back(UType("lui", 0b0110111));

    /** @brief AUIPC requires special handling during symbol resolution as compared to if no symbol is provided to
     * instruction.
     * - If no symbol; the provided immediate will be placed in the upper 20 bits of the instruction
     * - if symbol; the upper 20 bits of the provided resolved symbol will be placed in the upper 20 bits of the
     * instruction. To facilitate these two capabilities, we provide a symbol modifier functor to adjust any
     * provided symbol.
     */
    auto auipcSymbolModifier = [](uint32_t bareSymbol) { return bareSymbol >> 12; };
    instructions.push_back(std::shared_ptr<RVInstr>(
        new RVInstr(Opcode("auipc", {OpPart(0b0010111, 0, 6)}),
                    {std::make_shared<RVReg>(1, 7, 11),
                     std::make_shared<Imm>(2, 32, Imm::Repr::Hex, std::vector{ImmPart(0, 12, 31)},
                                           Imm::SymbolType::Absolute, auipcSymbolModifier)})));

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

void RV32I_Assembler::enableExtM(RVInstrVec& instructions, RVPseudoInstrVec&) const {
    // Pseudo-op functors
    // --

    // Assembler functors
    instructions.push_back(RType("mul", 0b000, 0b0000001));
    instructions.push_back(RType("mulh", 0b001, 0b0000001));
    instructions.push_back(RType("mulhsu", 0b010, 0b0000001));
    instructions.push_back(RType("mulhu", 0b011, 0b0000001));
    instructions.push_back(RType("div", 0b100, 0b0000001));
    instructions.push_back(RType("divu", 0b101, 0b0000001));
    instructions.push_back(RType("rem", 0b110, 0b0000001));
    instructions.push_back(RType("remu", 0b111, 0b0000001));
}

void RV32I_Assembler::enableExtF(RVInstrVec&, RVPseudoInstrVec&) const {
    // Pseudo-op functors

    // Assembler functors
}

}  // namespace AssemblerTmp
}  // namespace Ripes
