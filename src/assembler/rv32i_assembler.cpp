#include "rv32i_assembler.h"

#include <QByteArray>
#include <algorithm>

namespace Ripes {

namespace {
inline QByteArray uint32ToByteArr(uint32_t in) {
    const char bytes[] = {static_cast<char>(in & 0xff), static_cast<char>((in >> 8) & 0xff),
                          static_cast<char>((in >> 16) & 0xff), static_cast<char>((in >> 24) & 0xff)};
    return QByteArray::fromRawData(bytes, 4);
}

namespace instrType {
enum {
    LUI = 0b0110111,
    JAL = 0b1101111,
    JALR = 0b1100111,
    BRANCH = 0b1100011,
    LOAD = 0b0000011,
    STORE = 0b0100011,
    OP_IMM = 0b0010011,
    OP = 0b0110011,
    ECALL = 0b1110011,
    AUIPC = 0b0010111,
    INVALID = 0b0
};
}

const static QMap<QString, uint32_t> ABInames{
    {"zero", 0}, {"ra", 1},  {"sp", 2},   {"gp", 3},   {"tp", 4},  {"t0", 5},  {"t1", 6},  {"t2", 7},
    {"s0", 8},   {"a0", 10}, {"a1", 11},  {"a2", 12},  {"a3", 13}, {"a4", 14}, {"a5", 15}, {"a6", 16},
    {"a7", 17},  {"s1", 9},  {"s2", 18},  {"s3", 19},  {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23},
    {"s8", 24},  {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29}, {"t5", 30}, {"t6", 31}};

uint32_t getRegisterNumber(const QString& reg, bool& success) {
    // Converts a textual representation of a register to its numeric value
    QString regRes = reg;
    success = true;
    if (reg[0] == 'x') {
        regRes.remove('x');
        return regRes.toInt(&success, 10);
    } else if (ABInames.contains(reg)) {
        return ABInames[reg];
    }
    success = false;
    return 0;
}

/** @brief
 * Wraps a functor with the expectancy of a line containing @p N tokens.
 */
template <int N, typename T_funct, typename T_ret, typename... Args>
T_funct expectNTokens(const T_funct& functor) {
    return [=](const Assembler::SourceLine& line, Args... others) {
        if (line.tokens.size() != N) {
            return std::variant<Assembler::Error, T_ret>({Assembler::Error(
                line.source_line, "Expected " + QString::number(N) + " tokens but got " + line.tokens.size())});
        } else {
            return functor(line, others...);
        }
    };
}

template <uint32_t funct3, uint32_t funct7>
std::variant<Assembler::Error, QByteArray> assembleOpInstruction(const Assembler::SourceLine& line) {
    bool success;
    std::vector<uint32_t> regs(3);
    for (size_t token_idx = 1; token_idx <= regs.size(); token_idx++) {
        const auto& regToken = line.tokens[token_idx];
        const uint32_t reg = getRegisterNumber(regToken, success);
        if (!success) {
            return {Assembler::Error(line.source_line, "Unknown register '" + regToken + "'")};
        }
        regs.push_back(reg);
    }
    return uint32ToByteArr(instrType::OP | funct3 << 12 | funct7 << 25 | regs[0] << 7 | regs[1] << 15 | regs[2] << 20);
}

}  // namespace

RV32I_Assembler::RV32I_Assembler(const std::set<Extensions>& extensions) {
    enableExtI();
    for (const auto& extension : extensions) {
        switch (extension) {
            case Extensions::M:
                enableExtM();
                break;
            case Extensions::F:
                enableExtF();
                break;
            default:
                assert(false && "Unhandled ISA extension");
        }
    }
}

std::variant<Assembler::Error, std::optional<std::vector<Assembler::LineTokens>>>
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

std::variant<Assembler::Error, QByteArray>
RV32I_Assembler::assembleInstruction(const SourceLine& line, const Assembler::SymbolMap& symbols) const {
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

void RV32I_Assembler::enableExtI() {
    // Pseudo-op functors
    addPseudoOpExpanderFunctor("nop", expectNTokens<1, PseudoOpFunctor, PseudoOpFunctorRetT>([](const SourceLine&) {
                                   return std::vector<Assembler::LineTokens>{QString("addi x0 x0 0").split(' ')};
                               }));

    // Assembler functors
}

void RV32I_Assembler::enableExtM() {
    // Pseudo-op functors

    // Assembler functors

    addAssemblerFunctor("mul", expectNTokens<3, AssemblerFunctor, AssemblerFunctorRetT, const Assembler::SymbolMap&>(
                                   [](const SourceLine& line, const Assembler::SymbolMap&) {
                                       return assembleOpInstruction<0b000, 0b0000001>(line);
                                   }));

    addAssemblerFunctor("mulh", expectNTokens<3, AssemblerFunctor, AssemblerFunctorRetT, const Assembler::SymbolMap&>(
                                    [](const SourceLine& line, const Assembler::SymbolMap&) {
                                        return assembleOpInstruction<0b001, 0b0000001>(line);
                                    }));
    addAssemblerFunctor("mulhsu", expectNTokens<3, AssemblerFunctor, AssemblerFunctorRetT, const Assembler::SymbolMap&>(
                                      [](const SourceLine& line, const Assembler::SymbolMap&) {
                                          return assembleOpInstruction<0b011, 0b0000001>(line);
                                      }));
}
void RV32I_Assembler::enableExtF() {
    // Pseudo-op functors

    // Assembler functors
}

}  // namespace Ripes
