#include <QtTest/QTest>

#include "assembler/instruction.h"
#include "assembler/matcher.h"
#include "isainfo.h"

using namespace Ripes;
using namespace AssemblerTmp;
using RVISA = ISAInfo<ISA::RV32IM>;

class tst_Assembler : public QObject {
    Q_OBJECT

private slots:
    void tst_matcher();

private:
    std::vector<std::shared_ptr<Instruction<RVISA>>> createInstructions();
};

void tst_Assembler::tst_matcher() {
    auto matcher = AssemblerTmp::Matcher<RVISA>(createInstructions());

    std::vector<std::pair<QString, unsigned>> toMatch = {
        {"beq", 0b11111110000000000000111011100011},  {"addi", 0b00000111101100010000000010010011},
        {"slti", 0b00000000000100010010000100010011}, {"xori", 0b00000000000100010100000100010011},
        {"slli", 0b00000000000100010001000100010011}, {"srai", 0b01000000000100010101000100010011},
        {"add", 0b00000000001000010000000100110011},  {"sub", 0b01000000001000010000000100110011}};

    for (const auto& iter : toMatch) {
        auto match = matcher.matchInstruction(iter.second);
        try {
            auto& error = std::get<Error>(match);
            QFAIL(error.second.toStdString().c_str());
        } catch (const std::bad_variant_access&) {
        }
        auto matchInstr = std::get<const Instruction<RVISA>*>(match);
        if (matchInstr->name() != iter.first) {
            QString error =
                "Incorrect instruction decoded; got '" + matchInstr->name() + "' but expected '" + iter.first + "'";
            QFAIL(error.toStdString().c_str());
        }

        auto disRes = matchInstr->disassemble(iter.second, 0, nullptr);
        try {
            auto& error = std::get<Error>(disRes);
            QFAIL(error.second.toStdString().c_str());
        } catch (const std::bad_variant_access&) {
        }
        auto disassembled = std::get<LineTokens>(disRes);

        qDebug() << QString::number(iter.second, 2) << " = " << disassembled;
    }
}

#define BType(name, funct3)                                                                              \
    std::shared_ptr<Instruction<RVISA>>(new Instruction<RVISA>(                                          \
        Opcode(name, {OpPart(0b1100011, 0, 6), OpPart(funct3, 12, 14)}),                                 \
        {std::make_shared<Reg<RVISA>>(1, 15, 19), std::make_shared<Reg<RVISA>>(2, 20, 24),               \
         std::make_shared<Imm>(                                                                          \
             3, 13, Imm::Repr::Signed,                                                                   \
             std::vector{ImmPart(12, 31, 31), ImmPart(11, 7, 7), ImmPart(5, 25, 30), ImmPart(1, 8, 11)}, \
             Imm::SymbolType::Relative)}))

#define IType(name, funct3)                                                                                      \
    std::shared_ptr<Instruction<RVISA>>(                                                                         \
        new Instruction<RVISA>(Opcode(name, {OpPart(0b0010011, 0, 6), OpPart(funct3, 12, 14)}),                  \
                               {std::make_shared<Reg<RVISA>>(1, 7, 11), std::make_shared<Reg<RVISA>>(2, 15, 19), \
                                std::make_shared<Imm>(3, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define LoadType(name, funct3)                                                                                   \
    std::shared_ptr<Instruction<RVISA>>(                                                                         \
        new Instruction<RVISA>(Opcode(name, {OpPart(0b0000011, 0, 6), OpPart(funct3, 12, 14)}),                  \
                               {std::make_shared<Reg<RVISA>>(1, 7, 11), std::make_shared<Reg<RVISA>>(3, 15, 19), \
                                std::make_shared<Imm>(2, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

#define IShiftType(name, funct3, funct7)                                                         \
    std::shared_ptr<Instruction<RVISA>>(new Instruction<RVISA>(                                  \
        Opcode(name, {OpPart(0b0010011, 0, 6), OpPart(funct3, 12, 14), OpPart(funct7, 25, 31)}), \
        {std::make_shared<Reg<RVISA>>(1, 7, 11), std::make_shared<Reg<RVISA>>(2, 15, 19),        \
         std::make_shared<Imm>(3, 5, Imm::Repr::Unsigned, std::vector{ImmPart(0, 20, 24)})}))

#define RType(name, funct3, funct7)                                                              \
    std::shared_ptr<Instruction<RVISA>>(new Instruction<RVISA>(                                  \
        Opcode(name, {OpPart(0b0110011, 0, 6), OpPart(funct3, 12, 14), OpPart(funct7, 25, 31)}), \
        {std::make_shared<Reg<RVISA>>(1, 7, 11), std::make_shared<Reg<RVISA>>(2, 15, 19),        \
         std::make_shared<Reg<RVISA>>(3, 20, 24)}))

#define SType(name, funct3)                                                                                   \
    std::shared_ptr<Instruction<RVISA>>(new Instruction<RVISA>(                                               \
        Opcode(name, {OpPart(0b0100011, 0, 6), OpPart(funct3, 12, 14)}),                                      \
        {std::make_shared<Reg<RVISA>>(3, 15, 19),                                                             \
         std::make_shared<Imm>(2, 12, Imm::Repr::Signed, std::vector{ImmPart(5, 25, 31), ImmPart(0, 7, 11)}), \
         std::make_shared<Reg<RVISA>>(1, 20, 24)}))

#define UType(name, opcode)                                             \
    std::shared_ptr<Instruction<RVISA>>(                                \
        new Instruction<RVISA>(Opcode(name, {OpPart(opcode, 0, 6)}),    \
                               {std::make_shared<Reg<RVISA>>(1, 7, 11), \
                                std::make_shared<Imm>(2, 32, Imm::Repr::Hex, std::vector{ImmPart(12, 12, 31)})}))

#define JType(name, opcode)                                                                                  \
    std::shared_ptr<Instruction<RVISA>>(new Instruction<RVISA>(                                              \
        Opcode(name, {OpPart(opcode, 0, 6)}),                                                                \
        {std::make_shared<Reg<RVISA>>(1, 7, 11),                                                             \
         std::make_shared<Imm>(                                                                              \
             2, 21, Imm::Repr::Hex,                                                                          \
             std::vector{ImmPart(20, 31, 31), ImmPart(12, 12, 19), ImmPart(11, 20, 20), ImmPart(1, 21, 30)}, \
             Imm::SymbolType::Relative)}))

#define JALRType(name)                                                                                           \
    std::shared_ptr<Instruction<RVISA>>(                                                                         \
        new Instruction<RVISA>(Opcode(name, {OpPart(0b1100111, 0, 6), OpPart(0b000, 12, 14)}),                   \
                               {std::make_shared<Reg<RVISA>>(1, 7, 11), std::make_shared<Reg<RVISA>>(2, 15, 19), \
                                std::make_shared<Imm>(3, 12, Imm::Repr::Signed, std::vector{ImmPart(0, 20, 31)})}))

std::vector<std::shared_ptr<Instruction<RVISA>>> tst_Assembler::createInstructions() {
    std::vector<std::shared_ptr<Instruction<RVISA>>> instructions;
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

    return instructions;
}

QTEST_APPLESS_MAIN(tst_Assembler)
#include "tst_assembler.moc"
