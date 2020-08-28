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
        {"addi", 0b00000000000100010000000100010011}, {"slti", 0b00000000000100010010000100010011},
        {"xori", 0b00000000000100010100000100010011}, {"slli", 0b00000000000100010001000100010011},
        {"srai", 0b01000000000100010101000100010011}, {"add", 0b00000000001000010000000100110011},
        {"sub", 0b01000000001000010000000100110011}};

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

        auto disRes = matchInstr->disassemble(iter.second);
        try {
            auto& error = std::get<Error>(disRes);
            QFAIL(error.second.toStdString().c_str());
        } catch (const std::bad_variant_access&) {
        }
        auto disassembled = std::get<LineTokens>(disRes);

        qDebug() << QString::number(iter.second, 2) << " = " << disassembled;
    }
}

std::vector<std::shared_ptr<Instruction<RVISA>>> tst_Assembler::createInstructions() {
    auto addi = std::shared_ptr<Instruction<RVISA>>(
        new Instruction<RVISA>(Opcode<RVISA>("addi", {OpPart(0b0010011, 0, 6), OpPart(0b000, 12, 14)}), {}));
    auto slti = std::shared_ptr<Instruction<RVISA>>(
        new Instruction<RVISA>(Opcode<RVISA>("slti", {OpPart(0b0010011, 0, 6), OpPart(0b010, 12, 14)}), {}));
    auto xori = std::shared_ptr<Instruction<RVISA>>(
        new Instruction<RVISA>(Opcode<RVISA>("xori", {OpPart(0b0010011, 0, 6), OpPart(0b100, 12, 14)}), {}));
    auto slli = std::shared_ptr<Instruction<RVISA>>(new Instruction<RVISA>(
        Opcode<RVISA>("slli", {OpPart(0b0010011, 0, 6), OpPart(0b001, 12, 14), OpPart(0b0000000, 25, 31)}), {}));
    auto srai = std::shared_ptr<Instruction<RVISA>>(new Instruction<RVISA>(
        Opcode<RVISA>("srai", {OpPart(0b0010011, 0, 6), OpPart(0b101, 12, 14), OpPart(0b0100000, 25, 31)}), {}));
    auto add = std::shared_ptr<Instruction<RVISA>>(new Instruction<RVISA>(
        Opcode<RVISA>("add", {OpPart(0b0110011, 0, 6), OpPart(0b000, 12, 14), OpPart(0b0000000, 25, 31)}),
        {std::make_shared<Reg<RVISA>>(1, 7, 11), std::make_shared<Reg<RVISA>>(2, 15, 19),
         std::make_shared<Reg<RVISA>>(3, 20, 24)}));
    auto sub = std::shared_ptr<Instruction<RVISA>>(new Instruction<RVISA>(
        Opcode<RVISA>("sub", {OpPart(0b0110011, 0, 6), OpPart(0b000, 12, 14), OpPart(0b0100000, 25, 31)}),
        {std::make_shared<Reg<RVISA>>(1, 7, 11), std::make_shared<Reg<RVISA>>(2, 15, 19),
         std::make_shared<Reg<RVISA>>(3, 20, 24)}));

    return {addi, slti, xori, slli, srai, add, sub};
}

QTEST_APPLESS_MAIN(tst_Assembler)
#include "tst_assembler.moc"
