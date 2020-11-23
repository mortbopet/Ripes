#include <QtTest/QTest>

#include "assembler/instruction.h"
#include "assembler/matcher.h"
#include "isainfo.h"

#include "assembler/rv32i_assembler.h"

using namespace Ripes;
using namespace AssemblerTmp;
using RVISA = ISAInfo<ISA::RV32IM>;

class tst_Assembler : public QObject {
    Q_OBJECT

private slots:
    void tst_simpleprogram();
    void tst_simpleWithBranch();
    void tst_segment();
    void tst_matcher();
    void tst_label();
    void tst_labelWithPseudo();
    void tst_weirdImmediates();
    void tst_edgeImmediates();

private:
    std::vector<std::shared_ptr<Instruction<RVISA>>> createInstructions();
};

void tst_Assembler::tst_simpleprogram() {
    auto assembler = RV32I_Assembler({});
    QStringList program = QStringList() << "addi a0 a0 123"
                                        << "nop";
    auto res = assembler.assemble(program);
    if (res.errors.size() != 0) {
        res.errors.print();
        QFAIL("Errors during assembly");
    }
    auto disres = assembler.disassemble(res.program);

    return;
}

void tst_Assembler::tst_simpleWithBranch() {
    auto assembler = RV32I_Assembler({});
    QStringList program = QStringList() << "addi a0 a0 10"
                                        << "B:"
                                        << "addi a0 a0 -1"
                                        << "beqz a0 B";

    auto res = assembler.assemble(program);
    if (res.errors.size() != 0) {
        res.errors.print();
    }
    auto disres = assembler.disassemble(res.program);

    return;
}

void tst_Assembler::tst_weirdImmediates() {
    auto assembler = RV32I_Assembler({});
    QStringList program = QStringList() << "addi a0 a0 0q1234"
                                        << "addi a0 a0 -abcd"
                                        << "addi a0 a0 100000000"
                                        << "addi a0 a0 4096"   // too large
                                        << "addi a0 a0 2048"   // too large
                                        << "addi a0 a0 -2049"  // too large
                                        << "addi a0 a0 0xabcdabcdabcd";
    auto res = assembler.assemble(program);
    if (res.errors.size() != 0) {
        res.errors.print();
    }

    return;
}

void tst_Assembler::tst_edgeImmediates() {
    auto assembler = RV32I_Assembler({});
    QStringList program = QStringList() << "addi a0 a0 2047"
                                        << "addi a0 a0 -2048";
    auto res = assembler.assemble(program);
    if (res.errors.size() != 0) {
        res.errors.print();
        QFAIL("Expected no errors");
    }

    return;
}

void tst_Assembler::tst_label() {
    auto assembler = RV32I_Assembler({});
    QStringList program = QStringList() << "A:"
                                        << ""
                                        << "B: C:"
                                        << "D: E: addi a0 a0 -1";
    auto res = assembler.assemble(program);
    return;
}

void tst_Assembler::tst_segment() {
    auto assembler = RV32I_Assembler({});
    QStringList program = QStringList() << ".data nop"
                                        << "nop"
                                        << ".text .word"
                                        << "nop"
                                        << ".data"
                                        << "nop";
    auto res = assembler.assemble(program);
    if (res.errors.size() != 0) {
        res.errors.print();
    }
    return;
}

void tst_Assembler::tst_labelWithPseudo() {}

void tst_Assembler::tst_matcher() {
    auto assembler = RV32I_Assembler({});
    assembler.getMatcher().print();

    std::vector<std::pair<QString, unsigned>> toMatch = {
        {"beq", 0b11111110000000000000111011100011},  {"addi", 0b00000111101100010000000010010011},
        {"slti", 0b00000000000100010010000100010011}, {"xori", 0b00000000000100010100000100010011},
        {"slli", 0b00000000000100010001000100010011}, {"srai", 0b01000000000100010101000100010011},
        {"add", 0b00000000001000010000000100110011},  {"sub", 0b01000000001000010000000100110011}};

    for (const auto& iter : toMatch) {
        auto match = assembler.getMatcher().matchInstruction(iter.second);
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

        auto disRes = matchInstr->disassemble(iter.second, 0, {});
        try {
            auto& error = std::get<Error>(disRes);
            QFAIL(error.second.toStdString().c_str());
        } catch (const std::bad_variant_access&) {
        }
        auto disassembled = std::get<LineTokens>(disRes);

        qDebug() << QString::number(iter.second, 2) << " = " << disassembled;
    }
}

QTEST_APPLESS_MAIN(tst_Assembler)
#include "tst_assembler.moc"
