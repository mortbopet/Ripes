#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

/**
 * Extension enabler.
 * Calling an extension enabler will register the appropriate assemblers and
 * pseudo-op expander functors with the assembler. The extension enablers are
 * templated to allow for sharing implementations between 32- and 64-bit
 * variants.
 */
template <typename Reg__T>
struct RV_I {
  AssemblerTypes(Reg__T);
  enum class Options {
    shifts64BitVariant, // appends 'w' to 32-bit shift operations, for use in
                        // the 64-bit RISC-V ISA
    LI64BitVariant      // Modifies LI to be able to emit 64-bit constants
  };

  static void enable(const ISAInfoBase *isa, _InstrVec &instructions,
                     _PseudoInstrVec &pseudoInstructions,
                     const std::set<Options> &options = {}) {
    // Pseudo-op functors
    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new RVPseudoInstrLoad<Reg__T>(Token("lb"))));
    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new RVPseudoInstrLoad<Reg__T>(Token("lh"))));
    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new RVPseudoInstrLoad<Reg__T>(Token("lw"))));
    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new RVPseudoInstrStore<Reg__T>(Token("sb"))));
    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new RVPseudoInstrStore<Reg__T>(Token("sh"))));
    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new RVPseudoInstrStore<Reg__T>(Token("sw"))));

    // clang-format off
    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
        Token("la"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("auipc") << line.tokens.at(1) << Token(line.tokens.at(2), "%pcrel_hi"),
                                 LineTokens() << Token("addi") << line.tokens.at(1) << line.tokens.at(1) << Token(QString("(%1 + 4)").arg(line.tokens.at(2)), "%pcrel_lo")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
        Token("call"), {ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("auipc") << Token("x1") << Token(line.tokens.at(1), "%pcrel_hi"),
                                 LineTokens() << Token("jalr") << Token("x1") << Token("x1") << Token(QString("(%1 + 4)").arg(line.tokens.at(1)), "%pcrel_lo")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
        Token("tail"), {ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("auipc") << Token("x6") << Token(line.tokens.at(1), "%pcrel_hi"),
                                 LineTokens() << Token("jalr") << Token("x0") << Token("x6") << Token(QString("(%1 + 4)").arg(line.tokens.at(1)), "%pcrel_lo")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
        Token("j"), {ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("jal") << Token("x0") << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
        Token("jr"), {RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("jalr") << Token("x0") << line.tokens.at(1) << Token("0")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
        Token("jalr"), {RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("jalr") << Token("x1") << line.tokens.at(1) << Token("0")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
        Token("ret"), {}, [](const _PseudoInstruction&, const TokenizedSrcLine& , const SymbolMap&) {
            return LineTokensVec{LineTokens() << Token("jalr") << Token("x0") << Token("x1") << Token("0")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
        Token("jal"), {ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens() << Token("jal") << Token("x1") << line.tokens.at(1)};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token(Token("nop")), {}, [](const _PseudoInstruction&, const TokenizedSrcLine& , const SymbolMap&) {
            return LineTokensVec{LineTokens() << Token("addi") << Token("x0") << Token("x0") << Token("0")};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(
        Token("mv"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("addi"), line.tokens.at(1), line.tokens.at(2), Token("0")}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("not"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("xori"), line.tokens.at(1), line.tokens.at(2), Token("-1")}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("neg"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("sub"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("seqz"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("sltiu"), line.tokens.at(1), line.tokens.at(2), Token("1")}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("snez"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{ LineTokens{Token("sltu"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("sltz"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("slt"), line.tokens.at(1), line.tokens.at(2), Token("x0")}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("sgtz"), {RegTok, RegTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("slt"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("beqz"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("beq"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("bnez"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bne"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("blez"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bge"), Token("x0"), line.tokens.at(1), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("bgez"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bge"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("bltz"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("blt"), line.tokens.at(1), Token("x0"), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("bgtz"), {RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("blt"), Token("x0"), line.tokens.at(1), line.tokens.at(2)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("bgt"), {RegTok, RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("blt"), line.tokens.at(2), line.tokens.at(1), line.tokens.at(3)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("ble"), {RegTok, RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bge"), line.tokens.at(2), line.tokens.at(1), line.tokens.at(3)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("bgtu"), {RegTok, RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bltu"), line.tokens.at(2), line.tokens.at(1), line.tokens.at(3)}};
        })));

    pseudoInstructions.push_back(std::shared_ptr<_PseudoInstruction>(
        new _PseudoInstruction(Token("bleu"), {RegTok, RegTok, ImmTok}, _PseudoExpandFunc(line) {
            return LineTokensVec{LineTokens{Token("bgeu"), line.tokens.at(2), line.tokens.at(1), line.tokens.at(3)}};
        })));
    // clang-format on

    pseudoInstructions.push_back(std::shared_ptr<
                                 _PseudoInstruction>(new _PseudoInstruction(
        Token("li"), {RegTok, ImmTok}, _PseudoExpandFuncSyms(line, symbols) {
          LineTokensVec res;
          // Get an integer representation of the immediate, which might have
          // been a symbol.
          bool canConvert;
          bool liveDstReg = false;
          bool unsignedFitErr = false;
          int64_t immediate = getImmediateSext32(line.tokens.at(2), canConvert);

          if (!canConvert) {
            auto absSymbols = symbols.copyRelativeTo(line.sourceLine());
            // Check if the immediate has been made available in the symbol set
            // at this point...
            auto it = absSymbols.find(line.tokens.at(2));
            if (it != absSymbols.end()) {
              immediate = it->second;
            } else {
              if (unsignedFitErr) {
                return Result<std::vector<LineTokens>>{
                    Error(line, QString("Invalid immediate '%1'; can't emit "
                                        ">32-bit imm for non-RV64 target")
                                    .arg(line.tokens.at(2)))};
              } else {
                return Result<std::vector<LineTokens>>{Error(
                    line,
                    QString("Invalid immediate '%1'").arg(line.tokens.at(2)))};
              }
            }
          }

          /* The load-immediate pseudo instructions follows the LLVM
           * implementation, as seen here.
           * https://llvm.org/docs/doxygen/RISCVMatInt_8cpp_source.html
           * For more insight, please refer to there, since their comments have
           * been left out of this source code.
           */
          std::function<Result<std::vector<LineTokens>>(int64_t, bool)>
              genInstrSeq = [&](int64_t val, bool isRV64) {
                if (isInt<32>(val) || (!isRV64 && isUInt<32>(val))) {
                  int64_t Hi20 = ((val + 0x800) >> 12) & 0xFFFFF;
                  int64_t Lo12 = vsrtl::signextend<12>(val);
                  if (Hi20) {
                    res.push_back(LineTokens()
                                  << Token("lui") << line.tokens.at(1)
                                  << QString::number(Hi20));
                    liveDstReg = true;
                  }
                  if (Lo12 || Hi20 == 0) {
                    QString addiOpc = isRV64 && Hi20 != 0 ? "addiw" : "addi";
                    res.push_back(
                        LineTokens()
                        << Token(addiOpc) << line.tokens.at(1)
                        << (liveDstReg ? line.tokens.at(1) : Token("x0"))
                        << QString::number(Lo12));
                    liveDstReg = true;
                  }
                  return Result<std::vector<LineTokens>>{res};
                }
                if (!isRV64) {
                  return Result<std::vector<LineTokens>>{
                      Error(line, QString("Invalid immediate '%1'; can't emit "
                                          ">32-bit imm for non-RV64 target")
                                      .arg(line.tokens.at(2)))};
                }
                int64_t Lo12 = vsrtl::signextend<12>(val);
                int64_t Hi52 = ((VInt)val + 0x800ull) >> 12;
                int ShiftAmount = 12 + firstSetBitIdx(Hi52);
                Hi52 = vsrtl::signextend<int64_t>(Hi52 >> (ShiftAmount - 12),
                                                  64 - ShiftAmount);
                genInstrSeq(Hi52, isRV64);
                res.push_back(LineTokens() << Token("slli") << line.tokens.at(1)
                                           << line.tokens.at(1)
                                           << QString::number(ShiftAmount));
                if (Lo12) {
                  res.push_back(LineTokens()
                                << Token("addi") << line.tokens.at(1)
                                << line.tokens.at(1) << QString::number(Lo12));
                }
                return Result<std::vector<LineTokens>>{res};
              };
          auto instrSeq =
              genInstrSeq(immediate, options.count(Options::LI64BitVariant));
          return instrSeq;
        })));

    // Assembler functors

    instructions.push_back(std::shared_ptr<_Instruction>(new _Instruction(
        _Opcode(Token("ecall"),
                {OpPart(RVISA::Opcode::ECALL, 0, 6), OpPart(0, 7, 31)}),
        {})));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrUType<Reg__T>(Token("lui"), RVISA::Opcode::LUI, isa)));

    instructions.push_back(std::shared_ptr<_Instruction>(new _Instruction(
        _Opcode(Token("auipc"), {OpPart(RVISA::Opcode::AUIPC, 0, 6)}),
        {std::make_shared<_Reg>(isa, 1, 7, 11, "rd"),
         std::make_shared<_Imm>(2, 32, _Imm::Repr::Hex,
                                std::vector{ImmPart(0, 12, 31)},
                                _Imm::SymbolType::Absolute)})));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrJType<Reg__T>(Token("jal"), RVISA::Opcode::JAL, isa)));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrJALRType<Reg__T>(Token("jalr"), isa)));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrLType<Reg__T>(Token("lb"), 0b000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrLType<Reg__T>(Token("lh"), 0b001, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrLType<Reg__T>(Token("lw"), 0b010, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrLType<Reg__T>(Token("lbu"), 0b100, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrLType<Reg__T>(Token("lhu"), 0b101, isa)));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrSType<Reg__T>(Token("sb"), 0b000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrSType<Reg__T>(Token("sh"), 0b001, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrSType<Reg__T>(Token("sw"), 0b010, isa)));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrIType<Reg__T>(Token("addi"), 0b000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrIType<Reg__T>(Token("slti"), 0b010, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrIType<Reg__T>(Token("sltiu"), 0b011, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrIType<Reg__T>(Token("xori"), 0b100, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrIType<Reg__T>(Token("ori"), 0b110, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrIType<Reg__T>(Token("andi"), 0b111, isa)));

    if (options.count(Options::shifts64BitVariant)) {
      instructions.push_back(
          std::shared_ptr<_Instruction>(new RVInstrIShift32Type<Reg__T>(
              Token("slliw"), RVISA::OPIMM32, 0b001, 0b0000000, isa)));
      instructions.push_back(
          std::shared_ptr<_Instruction>(new RVInstrIShift32Type<Reg__T>(
              Token("srliw"), RVISA::OPIMM32, 0b101, 0b0000000, isa)));
      instructions.push_back(
          std::shared_ptr<_Instruction>(new RVInstrIShift32Type<Reg__T>(
              Token("sraiw"), RVISA::OPIMM32, 0b101, 0b0100000, isa)));
    } else {
      instructions.push_back(
          std::shared_ptr<_Instruction>(new RVInstrIShift32Type<Reg__T>(
              Token("slli"), RVISA::OPIMM, 0b001, 0b0000000, isa)));
      instructions.push_back(
          std::shared_ptr<_Instruction>(new RVInstrIShift32Type<Reg__T>(
              Token("srli"), RVISA::OPIMM, 0b101, 0b0000000, isa)));
      instructions.push_back(
          std::shared_ptr<_Instruction>(new RVInstrIShift32Type<Reg__T>(
              Token("srai"), RVISA::OPIMM, 0b101, 0b0100000, isa)));
    }

    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("add"), 0b000, 0b0000000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("sub"), 0b000, 0b0100000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("sll"), 0b001, 0b0000000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("slt"), 0b010, 0b0000000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("sltu"), 0b011, 0b0000000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("xor"), 0b100, 0b0000000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("srl"), 0b101, 0b0000000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("sra"), 0b101, 0b0100000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("or"), 0b110, 0b0000000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrRType<Reg__T>(Token("and"), 0b111, 0b0000000, isa)));

    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrBType<Reg__T>(Token("beq"), 0b000, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrBType<Reg__T>(Token("bne"), 0b001, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrBType<Reg__T>(Token("blt"), 0b100, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrBType<Reg__T>(Token("bge"), 0b101, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrBType<Reg__T>(Token("bltu"), 0b110, isa)));
    instructions.push_back(std::shared_ptr<_Instruction>(
        new RVInstrBType<Reg__T>(Token("bgeu"), 0b111, isa)));
  }
};

} // namespace Assembler
} // namespace Ripes
