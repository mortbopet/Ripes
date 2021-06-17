#include "rv32i_assembler.h"
#include "gnudirectives.h"
#include "ripessettings.h"
#include "rvassembler_common.h"
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

void RV32I_Assembler::enableExtI(const ISAInfoBase* isa, InstrVec& instructions, PseudoInstrVec& pseudoInstructions,
                                 const std::set<Options>& options) {
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

    pseudoInstructions.push_back(std::shared_ptr<PseudoInstruction>(new PseudoInstruction(
        Token("li"), {RegTok, ImmTok}, _PseudoExpandFuncSyms(line, symbols) {
            LineTokensVec res;
            // Get an integer representation of the immediate, which might have been a symbol.
            bool canConvert;
            bool liveDstReg = false;
            int64_t immediate = getImmediate(line.tokens.at(2), canConvert);

            if (!canConvert) {
                // Check if the immediate has been made available in the symbol set at this point...
                auto it = symbols.find(line.tokens.at(2));
                if (it != symbols.end()) {
                    immediate = it->second;
                } else {
                    return PseudoExpandRes{
                        Error(line.sourceLine, QString("Invalid immediate '%1'").arg(line.tokens.at(2)))};
                }
            }

            /* The load-immediate pseudo instructions follows the LLVM implementation, as seen here.
             * https://llvm.org/docs/doxygen/RISCVMatInt_8cpp_source.html
             * For more insight, please refer to there, since their comments have been left out of this source
             * code.
             */
            std::function<PseudoExpandRes(int64_t, bool)> genInstrSeq = [&](int64_t val, bool isRV64) {
                // LLVM has preconversion of all immediates into a sign-extended 64-bit version, and only checks
                // int32. getImmediate, as used above, does not sign-extend unsigned immediates. Therefore we must
                // go into the terminal case when @p val is either representable as uint32 or int32.
                if (isUInt<32>(val) || isInt<32>(val)) {
                    int64_t Hi20 = ((val + 0x800) >> 12) & 0xFFFFF;
                    int64_t Lo12 = signextend<int64_t, 12>(val);
                    if (Hi20) {
                        res.push_back(LineTokens() << Token("lui") << line.tokens.at(1) << QString::number(Hi20));
                        liveDstReg = true;
                    }
                    if (Lo12 || Hi20 == 0) {
                        QString addiOpc = isRV64 && Hi20 != 0 ? "addiw" : "addi";
                        res.push_back(LineTokens()
                                      << Token(addiOpc) << line.tokens.at(1)
                                      << (liveDstReg ? line.tokens.at(1) : Token("x0")) << QString::number(Lo12));
                        liveDstReg = true;
                    }
                    return PseudoExpandRes{res};
                }
                if (!isRV64) {
                    return PseudoExpandRes{Error(
                        line.sourceLine, QString("Invalid immediate '%1'; can't emit >32-bit imm for non-RV64 target")
                                             .arg(line.tokens.at(2)))};
                }
                int64_t Lo12 = signextend<int64_t, 12>(val);
                int64_t Hi52 = ((uint64_t)val + 0x800ull) >> 12;
                int ShiftAmount = 12 + firstSetBitIdx(Hi52);
                Hi52 = signextend<int64_t>(Hi52 >> (ShiftAmount - 12), 64 - ShiftAmount);
                genInstrSeq(Hi52, isRV64);
                res.push_back(LineTokens() << Token("slli") << line.tokens.at(1) << line.tokens.at(1)
                                           << QString::number(ShiftAmount));
                if (Lo12) {
                    res.push_back(LineTokens()
                                  << Token("addi") << line.tokens.at(1) << line.tokens.at(1) << QString::number(Lo12));
                }
                return PseudoExpandRes{res};
            };
            auto instrSeq = genInstrSeq(immediate, options.count(Options::LI64BitVariant));
            return instrSeq;
        })));

    // Assembler functors

    instructions.push_back(std::shared_ptr<Instruction>(
        new Instruction(Opcode(Token("ecall"), {OpPart(RVISA::Opcode::ECALL, 0, 6), OpPart(0, 7, 31)}), {})));

    instructions.push_back(UType(Token("lui"), RVISA::Opcode::LUI));

    instructions.push_back(std::shared_ptr<Instruction>(new Instruction(
        Opcode(Token("auipc"), {OpPart(RVISA::Opcode::AUIPC, 0, 6)}),
        {std::make_shared<Reg>(isa, 1, 7, 11, "rd"),
         std::make_shared<Imm>(2, 32, Imm::Repr::Hex, std::vector{ImmPart(0, 12, 31)}, Imm::SymbolType::Absolute)})));

    instructions.push_back(JType(Token("jal"), RVISA::Opcode::JAL));

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

    if (options.count(Options::shifts64BitVariant)) {
        instructions.push_back(IShiftType32(Token("slliw"), RVISA::OPIMM32, 0b001, 0b0000000));
        instructions.push_back(IShiftType32(Token("srliw"), RVISA::OPIMM32, 0b101, 0b0000000));
        instructions.push_back(IShiftType32(Token("sraiw"), RVISA::OPIMM32, 0b101, 0b0100000));
    } else {
        instructions.push_back(IShiftType32(Token("slli"), RVISA::OPIMM, 0b001, 0b0000000));
        instructions.push_back(IShiftType32(Token("srli"), RVISA::OPIMM, 0b101, 0b0000000));
        instructions.push_back(IShiftType32(Token("srai"), RVISA::OPIMM, 0b101, 0b0100000));
    }

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

void RV32I_Assembler::enableExtM(const ISAInfoBase* isa, InstrVec& instructions, PseudoInstrVec&) {
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

void RV32I_Assembler::enableExtF(const ISAInfoBase*, InstrVec&, PseudoInstrVec&) {
    // Pseudo-op functors

    // Assembler functors
}

}  // namespace Assembler
}  // namespace Ripes
