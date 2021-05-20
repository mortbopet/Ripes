#include "rv32i_assembler.h"
#include "gnudirectives.h"
#include "ripessettings.h"
#include "rvrelocations.h"

#include <QByteArray>
#include <algorithm>

namespace Ripes {
namespace Assembler {

RV32I_Assembler::RV32I_Assembler(const ISAInfo<ISA::RV32I>* isa) : Assembler(isa) {
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

std::tuple<InstrVec, PseudoInstrVec> RV32I_Assembler::initInstructions(const ISAInfo<ISA::RV32I>* isa) const {
    InstrVec instructions;
    PseudoInstrVec pseudoInstructions;

    enableExtI(isa, instructions, pseudoInstructions);
    for (const auto& extension : isa->enabledExtensions()) {
        switch (extension.unicode()->toLatin1()) {
            case 'M':
                enableExtM(isa, instructions, pseudoInstructions);
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

#define BType(name, funct3)                                                                              \
    std::shared_ptr<Instruction>(new Instruction(                                                        \
        Opcode(name, {OpPart(0b1100011, 0, 6), OpPart(funct3, 12, 14)}),                                 \
        {std::make_shared<Reg>(isa, 1, 15, 19, "rs1"), std::make_shared<Reg>(isa, 2, 20, 24, "rs2"),     \
         std::make_shared<Imm>(                                                                          \
             3, 13, Imm::Repr::Signed,                                                                   \
             std::vector{ImmPart(12, 31, 31), ImmPart(11, 7, 7), ImmPart(5, 25, 30), ImmPart(1, 8, 11)}, \
             Imm::SymbolType::Relative)}))

#define IType(name, funct3)                                                                                        \
    std::shared_ptr<Instruction>(                                                                                  \
        new Instruction(Opcode(name, {OpPart(0b0010011, 0, 6), OpPart(funct3, 12, 14)}),                           \
                        {std::make_shared<Reg>(isa, 1, 7, 11, "rd"), std::make_shared<Reg>(isa, 2, 15, 19, "rs1"), \
                         std::make_shared<Imm>(3, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define LoadType(name, funct3)                                                                                     \
    std::shared_ptr<Instruction>(                                                                                  \
        new Instruction(Opcode(name, {OpPart(0b0000011, 0, 6), OpPart(funct3, 12, 14)}),                           \
                        {std::make_shared<Reg>(isa, 1, 7, 11, "rd"), std::make_shared<Reg>(isa, 3, 15, 19, "rs1"), \
                         std::make_shared<Imm>(2, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define IShiftType(name, funct3, funct7)                                                                           \
    std::shared_ptr<Instruction>(                                                                                  \
        new Instruction(Opcode(name, {OpPart(0b0010011, 0, 6), OpPart(funct3, 12, 14), OpPart(funct7, 25, 31)}),   \
                        {std::make_shared<Reg>(isa, 1, 7, 11, "rd"), std::make_shared<Reg>(isa, 2, 15, 19, "rs1"), \
                         std::make_shared<Imm>(3, 5, Imm::Repr::Unsigned, std::vector{ImmPart(0, 20, 24)})}))

#define RType(name, funct3, funct7)                                                                                \
    std::shared_ptr<Instruction>(                                                                                  \
        new Instruction(Opcode(name, {OpPart(0b0110011, 0, 6), OpPart(funct3, 12, 14), OpPart(funct7, 25, 31)}),   \
                        {std::make_shared<Reg>(isa, 1, 7, 11, "rd"), std::make_shared<Reg>(isa, 2, 15, 19, "rs1"), \
                         std::make_shared<Reg>(isa, 3, 20, 24, "rs2")}))

#define SType(name, funct3)                                                                                   \
    std::shared_ptr<Instruction>(new Instruction(                                                             \
        Opcode(name, {OpPart(0b0100011, 0, 6), OpPart(funct3, 12, 14)}),                                      \
        {std::make_shared<Reg>(isa, 3, 15, 19, "rs1"),                                                        \
         std::make_shared<Imm>(2, 12, Imm::Repr::Signed, std::vector{ImmPart(5, 25, 31), ImmPart(0, 7, 11)}), \
         std::make_shared<Reg>(isa, 1, 20, 24, "rs2")}))

#define UType(name, opcode)                                          \
    std::shared_ptr<Instruction>(                                    \
        new Instruction(Opcode(name, {OpPart(opcode, 0, 6)}),        \
                        {std::make_shared<Reg>(isa, 1, 7, 11, "rd"), \
                         std::make_shared<Imm>(2, 32, Imm::Repr::Hex, std::vector{ImmPart(0, 12, 31)})}))

#define JType(name, opcode)                                                                                  \
    std::shared_ptr<Instruction>(new Instruction(                                                            \
        Opcode(name, {OpPart(opcode, 0, 6)}),                                                                \
        {std::make_shared<Reg>(isa, 1, 7, 11, "rd"),                                                         \
         std::make_shared<Imm>(                                                                              \
             2, 21, Imm::Repr::Signed,                                                                       \
             std::vector{ImmPart(20, 31, 31), ImmPart(12, 12, 19), ImmPart(11, 20, 20), ImmPart(1, 21, 30)}, \
             Imm::SymbolType::Relative)}))

#define JALRType(name)                                                                                             \
    std::shared_ptr<Instruction>(                                                                                  \
        new Instruction(Opcode(name, {OpPart(0b1100111, 0, 6), OpPart(0b000, 12, 14)}),                            \
                        {std::make_shared<Reg>(isa, 1, 7, 11, "rd"), std::make_shared<Reg>(isa, 2, 15, 19, "rs1"), \
                         std::make_shared<Imm>(3, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define RegTok PseudoInstruction::reg()
#define ImmTok PseudoInstruction::imm()
#define CreatePseudoInstruction
#define _PseudoExpandFuncSyms(line, symbols) \
    [](const PseudoInstruction&, const TokenizedSrcLine& line, const SymbolMap& symbols)

#define _PseudoExpandFunc(line) [](const PseudoInstruction&, const TokenizedSrcLine& line, const SymbolMap&)

#define PseudoLoad(name)                                                                                               \
    std::shared_ptr<PseudoInstruction>(new PseudoInstruction(                                                          \
        name, {RegTok, ImmTok}, _PseudoExpandFunc(line) {                                                              \
            LineTokensVec v;                                                                                           \
            v.push_back(LineTokens() << Token("auipc") << line.tokens.at(1) << Token(line.tokens.at(2), "%pcrel_hi")); \
            v.push_back(LineTokens() << name << line.tokens.at(1)                                                      \
                                     << Token(QString("(%1 + 4)").arg(line.tokens.at(2)), "%pcrel_lo")                 \
                                     << line.tokens.at(1));                                                            \
            return v;                                                                                                  \
        }))

// The sw is a pseudo-op if a symbol is given as the immediate token. Thus, if we detect that
// a number has been provided, then abort the pseudo-op handling.
#define PseudoStore(name)                                                                                              \
    std::shared_ptr<PseudoInstruction>(new PseudoInstruction(                                                          \
        name, {RegTok, ImmTok, RegTok}, _PseudoExpandFunc(line) {                                                      \
            bool canConvert;                                                                                           \
            getImmediate(line.tokens.at(2), canConvert);                                                               \
            if (canConvert) {                                                                                          \
                return PseudoExpandRes(Error(0, "Unused; will fallback to non-pseudo op sw"));                         \
            }                                                                                                          \
            LineTokensVec v;                                                                                           \
            v.push_back(LineTokens() << Token("auipc") << line.tokens.at(3) << Token(line.tokens.at(2), "%pcrel_hi")); \
            v.push_back(LineTokens() << name << line.tokens.at(1)                                                      \
                                     << Token(QString("(%1 + 4)").arg(line.tokens.at(2)), "%pcrel_lo")                 \
                                     << line.tokens.at(3));                                                            \
            return PseudoExpandRes(v);                                                                                 \
        }))

void RV32I_Assembler::enableExtI(const ISAInfo<ISA::RV32I>* isa, InstrVec& instructions,
                                 PseudoInstrVec& pseudoInstructions) const {
    // Pseudo-op functors
    pseudoInstructions.push_back(PseudoLoad(Token("lb")));
    pseudoInstructions.push_back(PseudoLoad(Token("lh")));
    pseudoInstructions.push_back(PseudoLoad(Token("lw")));
    pseudoInstructions.push_back(PseudoStore(Token("sb")));
    pseudoInstructions.push_back(PseudoStore(Token("sh")));
    pseudoInstructions.push_back(PseudoStore(Token("sw")));

    // clang-format off
    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("la"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("auipc") << line.tokens.at(1) << Token(line.tokens.at(2), "%pcrel_hi"),
                                 LineTokens() << Token("addi") << line.tokens.at(1) << line.tokens.at(1) << Token(QString("(%1 + 4)").arg(line.tokens.at(2)), "%pcrel_lo")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("call"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("auipc") << Token("x6") << line.tokens.at(1),
                                 LineTokens() << Token("jalr") << Token("x1") << Token("x6") << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("tail"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("auipc") << Token("x6") << line.tokens.at(1),
                                 LineTokens() << Token("jalr") << Token("x0") << Token("x6") << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("j"), {ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("jal") << Token("x0") << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("jr"), {RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("jalr") << Token("x0") << line.tokens.at(1) << Token("0")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("jalr"), {RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("jalr") << Token("x1") << line.tokens.at(1) << Token("0")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("ret"), {}, [](const PseudoInstruction&, const TokenizedSrcLine& , const SymbolMap&) {
            return LineTokensVec{LineTokens() << Token("jalr") << Token("x0") << Token("x1") << Token("0")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("jal"), {ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("jal") << Token("x1") << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("li"), {RegTok, ImmTok},
        _PseudoExpandFuncSyms(line, symbols) {
            LineTokensVec v;
            // Determine whether an ADDI or LUI instruction is sufficient, or if both LUI and ADDI is needed, by
            // analysing the immediate size
            bool canConvert;
            int immediate = getImmediate(line.tokens.at(2), canConvert);

           if(!canConvert) {
                // Check if the immediate has been made available in the symbol set at this point...
                auto it = symbols.find(line.tokens.at(2));
                if(it != symbols.end()){
                    immediate = it->second;
                } else {
                    return PseudoExpandRes{Error(line.sourceLine, QString("Invalid immediate '%1'").arg(line.tokens.at(2)))};
                }
            }

            // Generate offset required for discerning between positive and negative immediates
            if (isInt<12>(immediate)) {
                // immediate can be represented by 12 bits, ADDI is sufficient
                v.push_back(LineTokens() << Token("addi") << line.tokens.at(1) << Token("x0") << QString::number(immediate));
            } else {
                const int lower12Signed = signextend<int32_t, 12>(immediate & 0xFFF);
                int signOffset = lower12Signed < 0 ? 1 : 0;
                v.push_back(LineTokens() << Token("lui") << line.tokens.at(1)
                                          << QString::number((static_cast<uint32_t>(immediate) >> 12) + signOffset));
                if ((immediate & 0xFFF) != 0) {
                    v.push_back(LineTokens()
                                << Token("addi") << line.tokens.at(1) << line.tokens.at(1) << QString::number(lower12Signed));
                }
            }
            return PseudoExpandRes{v};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token(Token("nop")), {}, [](const PseudoInstruction&, const TokenizedSrcLine& , const SymbolMap&) {
            return LineTokensVec{LineTokens() << Token("addi") << Token("x0") << Token("x0") << Token("0")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("mv"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("addi"), line.tokens.at(1), line.tokens.at(2), Token("0")}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("not"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("xori"), line.tokens.at(1), line.tokens.at(2), Token("-1")}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("neg"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("sub"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("seqz"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("sltiu"), line.tokens.at(1), line.tokens.at(2), Token("1")}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("snez"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{ LineTokens{Token("sltu"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("sltz"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("slt"), line.tokens.at(1), line.tokens.at(2), Token("x0")}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("sgtz"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("slt"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("beqz"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("beq"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("bnez"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bne"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("blez"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bge"), Token("x0"), line.tokens.at(1), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("bgez"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bge"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("bltz"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("blt"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("bgtz"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("blt"), Token("x0"), line.tokens.at(1), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("bgt"), {RegTok, RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("blt"), line.tokens.at(2), line.tokens.at(1), line.tokens.at(3)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("ble"), {RegTok, RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bge"), line.tokens.at(2), line.tokens.at(1), line.tokens.at(3)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("bgtu"), {RegTok, RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bltu"), line.tokens.at(2), line.tokens.at(2), line.tokens.at(3)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(
        new PseudoInstruction(Token("bleu"), {RegTok, RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bgeu"), line.tokens.at(2), line.tokens.at(2), line.tokens.at(3)}};
        })));
    // clang-format on

    // Assembler functors

    instructions.push_back(std::shared_ptr<Instruction>(
        new Instruction(Opcode(Token("ecall"), {OpPart(0b1110011, 0, 6), OpPart(0, 7, 31)}), {})));

    instructions.push_back(UType(Token("lui"), 0b0110111));

    instructions.push_back(std::shared_ptr<Instruction>(new Instruction(
        Opcode(Token("auipc"), {OpPart(0b0010111, 0, 6)}),
        {std::make_shared<Reg>(isa, 1, 7, 11, "rd"),
         std::make_shared<Imm>(2, 32, Imm::Repr::Hex, std::vector{ImmPart(0, 12, 31)}, Imm::SymbolType::Absolute)})));

    instructions.push_back(JType(Token("jal"), 0b1101111));

    instructions.push_back(JALRType(Token("jalr")));

    instructions.push_back(LoadType(Token("lb"), 0b000));
    instructions.push_back(LoadType(Token("lh"), 0b001));
    instructions.push_back(LoadType(Token("lw"), 0b010));
    instructions.push_back(LoadType(Token("lbu"), 0b100));
    instructions.push_back(LoadType(Token("lhu"), 0b101));

    instructions.push_back(SType(Token("sb"), 0b000));
    instructions.push_back(SType(Token("sh"), 0b001));
    instructions.push_back(SType(Token("sw"), 0b010));

    instructions.push_back(IType(Token("addi"), 0b000));
    instructions.push_back(IType(Token("slti"), 0b010));
    instructions.push_back(IType(Token("sltiu"), 0b011));
    instructions.push_back(IType(Token("xori"), 0b100));
    instructions.push_back(IType(Token("ori"), 0b110));
    instructions.push_back(IType(Token("andi"), 0b111));

    instructions.push_back(IShiftType(Token("slli"), 0b001, 0b0000000));
    instructions.push_back(IShiftType(Token("srli"), 0b101, 0b0000000));
    instructions.push_back(IShiftType(Token("srai"), 0b101, 0b0100000));

    instructions.push_back(RType(Token("add"), 0b000, 0b0000000));
    instructions.push_back(RType(Token("sub"), 0b000, 0b0100000));
    instructions.push_back(RType(Token("sll"), 0b001, 0b0000000));
    instructions.push_back(RType(Token("slt"), 0b010, 0b0000000));
    instructions.push_back(RType(Token("sltu"), 0b011, 0b0000000));
    instructions.push_back(RType(Token("xor"), 0b100, 0b0000000));
    instructions.push_back(RType(Token("srl"), 0b101, 0b0000000));
    instructions.push_back(RType(Token("sra"), 0b101, 0b0100000));
    instructions.push_back(RType(Token("or"), 0b110, 0b0000000));
    instructions.push_back(RType(Token("and"), 0b111, 0b0000000));

    instructions.push_back(BType(Token("beq"), 0b000));
    instructions.push_back(BType(Token("bne"), 0b001));
    instructions.push_back(BType(Token("blt"), 0b100));
    instructions.push_back(BType(Token("bge"), 0b101));
    instructions.push_back(BType(Token("bltu"), 0b110));
    instructions.push_back(BType(Token("bgeu"), 0b111));
}

void RV32I_Assembler::enableExtM(const ISAInfo<ISA::RV32I>* isa, InstrVec& instructions, PseudoInstrVec&) const {
    // Pseudo-op functors
    // --

    // Assembler functors
    instructions.push_back(RType(Token("mul"), 0b000, 0b0000001));
    instructions.push_back(RType(Token("mulh"), 0b001, 0b0000001));
    instructions.push_back(RType(Token("mulhsu"), 0b010, 0b0000001));
    instructions.push_back(RType(Token("mulhu"), 0b011, 0b0000001));
    instructions.push_back(RType(Token("div"), 0b100, 0b0000001));
    instructions.push_back(RType(Token("divu"), 0b101, 0b0000001));
    instructions.push_back(RType(Token("rem"), 0b110, 0b0000001));
    instructions.push_back(RType(Token("remu"), 0b111, 0b0000001));
}

void RV32I_Assembler::enableExtF(const ISAInfo<ISA::RV32I>*, InstrVec&, PseudoInstrVec&) const {
    // Pseudo-op functors

    // Assembler functors
}

}  // namespace Assembler
}  // namespace Ripes
