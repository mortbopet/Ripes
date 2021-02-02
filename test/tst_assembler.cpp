#include <QtTest/QTest>

#include "assembler/instruction.h"
#include "assembler/matcher.h"
#include "isa/isainfo.h"
#include "isa/rv32isainfo.h"

#include "assembler/rv32i_assembler.h"

const QString s_testdir = VSRTL_RISCV_TEST_DIR;

using namespace Ripes;
using namespace Assembler;
using RVISA = ISAInfo<ISA::RV32I>;

class tst_Assembler : public QObject {
    Q_OBJECT

private slots:
    void tst_riscv();
    void tst_simpleprogram();
    void tst_simpleWithBranch();
    void tst_segment();
    void tst_matcher();
    void tst_label();
    void tst_labelWithPseudo();
    void tst_weirdImmediates();
    void tst_weirdDirectives();
    void tst_edgeImmediates();
    void tst_benchmarkNew();
    void tst_invalidreg();
    void tst_expression();
    void tst_invalidLabel();

private:
    std::vector<std::shared_ptr<Instruction>> createInstructions();

    QString createProgram(int entries) {
        QString out;
        out += ".data\n";
        for (int i = 0; i < entries; i++) {
            out += "L" + QString::number(i) + ": .word 1 2 3 4\n";
        }
        out += ".text\n";
        for (int i = 0; i < entries; i++) {
            out += "LA" + QString::number(i) + ": addi a0 a0 1\n";
            out += "nop\n";
            out += "beqz a0 LA" + QString::number(i) + "\n";
        }
        return out;
    }

    enum class Expect { Fail, Success };
    void testAssemble(const QStringList& program, Expect expect) {
        auto isa = std::make_unique<ISAInfo<ISA::RV32I>>(QStringList());
        auto assembler = RV32I_Assembler(isa.get());
        auto res = assembler.assemble(program);
        if ((res.errors.size() != 0) ^ (expect == Expect::Fail)) {
            res.errors.print();
            QString err;
            QString failExpectString = (expect == Expect::Fail ? "fail" : "success");
            err += "Expected " + failExpectString + " on program: \n";
            err += program.join('\n');
            QFAIL(err.toStdString().c_str());
        }
    }
};

void tst_Assembler::tst_riscv() {
    // Tests all of the available RISC-V assembly programs
    const auto dir = QDir(s_testdir);
    const auto testFiles = dir.entryList({"*.s"});

    auto testFunct = [](const QString& filename) {
        auto isa = std::make_unique<ISAInfo<ISA::RV32I>>(QStringList());
        auto assembler = RV32I_Assembler(isa.get());
        auto f = QFile(filename);
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        auto program = QString(f.readAll());
        auto res = assembler.assembleRaw(program);
        if (res.errors.size() != 0) {
            res.errors.print();
            auto errmsg = filename + ": error during assembling!";
            // QFAIL(errmsg.toStdString().c_str());
        }
    };

    for (const auto& test : testFiles) {
        if (test.startsWith("f"))
            continue;  // skip float tests
        testFunct(s_testdir + QDir::separator() + test);
    }
}

void tst_Assembler::tst_expression() {
    testAssemble(QStringList() << ".data"
                               << "B: .word 123"
                               << ".text"
                               << "lw x10 (B + (4* 3))(x10)",
                 Expect::Success);
    testAssemble(QStringList() << ".data"
                               << "A: .word 1"
                               << ".text"
                               << "lw a0 A(+1) a0",
                 Expect::Fail);

    return;
}

void tst_Assembler::tst_invalidLabel() {
    testAssemble(QStringList() << ".text"
                               << "ABC+: lw x10 ABC+ x10",
                 Expect::Fail);
    testAssemble(QStringList() << "a: lw a0 a+ a0", Expect::Fail);
    testAssemble(QStringList() << "addi a0 a0 (a", Expect::Fail);
}

void tst_Assembler::tst_benchmarkNew() {
    auto isa = std::make_unique<ISAInfo<ISA::RV32I>>(QStringList());
    auto assembler = RV32I_Assembler(isa.get());
    auto program = createProgram(1000);
    QBENCHMARK { assembler.assembleRaw(program); }
}

void tst_Assembler::tst_simpleprogram() {
    testAssemble(QStringList() << ".data"
                               << "B: .word 1, 2, 2"
                               << "C: .string \"hello world!\""
                               << ".text"
                               << "addi a0 a0 123 # Hello world"
                               << "nop",
                 Expect::Success);
}

void tst_Assembler::tst_simpleWithBranch() {
    testAssemble(QStringList() << "B:nop"
                               << "sw x0, 24(sp) # tmp. res 2"
                               << "addi a0 a0 10"
                               << "addi a0 a0 -1"
                               << "beqz a0 B",
                 Expect::Success);
}

void tst_Assembler::tst_weirdImmediates() {
    testAssemble(QStringList() << "addi a0 a0 0q1234"
                               << "addi a0 a0 -abcd"
                               << "addi a0 a0 100000000"
                               << "addi a0 a0 4096"   // too large
                               << "addi a0 a0 2048"   // too large
                               << "addi a0 a0 -2049"  // too large
                               << "addi a0 a0 0xabcdabcdabcd",
                 Expect::Fail);
}

void tst_Assembler::tst_weirdDirectives() {
    testAssemble(QStringList() << ".text"
                               << "B: .a"
                               << ""
                               << ".c"
                               << "nop",
                 Expect::Fail);
    // Test that a directive which requires no arguments throws error
    testAssemble(QStringList{".data foo"}, Expect::Fail);
}

void tst_Assembler::tst_invalidreg() {
    testAssemble(QStringList() << "addi x36 x46 1", Expect::Fail);
}

void tst_Assembler::tst_edgeImmediates() {
    testAssemble(QStringList() << "addi a0 a0 2047"
                               << "addi a0 a0 -2048",
                 Expect::Success);
}

void tst_Assembler::tst_label() {
    testAssemble(QStringList() << "A:"
                               << ""
                               << "B: C:"
                               << "D: E: addi a0 a0 -1",
                 Expect::Success);
}

void tst_Assembler::tst_segment() {
    testAssemble(QStringList() << ".data"
                               << "nop"
                               << ".text "
                               << "L: .word 1, 2, 3 ,4"
                               << "nop"
                               << ".data"
                               << "nop",
                 Expect::Success);
}

void tst_Assembler::tst_labelWithPseudo() {
    testAssemble(QStringList() << "j end"
                               << "end:nop",
                 Expect::Success);
}

void tst_Assembler::tst_matcher() {
    auto isa = std::make_unique<ISAInfo<ISA::RV32I>>(QStringList());
    auto assembler = RV32I_Assembler(isa.get());
    assembler.getMatcher().print();

    std::vector<std::pair<QString, unsigned>> toMatch = {
        {"beq", 0b11111110000000000000111011100011},  {"addi", 0b00000111101100010000000010010011},
        {"slti", 0b00000000000100010010000100010011}, {"xori", 0b00000000000100010100000100010011},
        {"slli", 0b00000000000100010001000100010011}, {"srai", 0b01000000000100010101000100010011},
        {"add", 0b00000000001000010000000100110011},  {"sub", 0b01000000001000010000000100110011}};

    for (const auto& iter : toMatch) {
        auto match = assembler.getMatcher().matchInstruction(iter.second);
        if (auto* error = std::get_if<Error>(&match)) {
            QFAIL(error->second.toStdString().c_str());
        }

        auto matchInstr = std::get<const Instruction*>(match);
        if (matchInstr->name() != iter.first) {
            QString error =
                "Incorrect instruction decoded; got '" + matchInstr->name() + "' but expected '" + iter.first + "'";
            QFAIL(error.toStdString().c_str());
        }

        auto disRes = matchInstr->disassemble(iter.second, 0, {});
        if (auto* error = std::get_if<Error>(&disRes)) {
            QFAIL(error->second.toStdString().c_str());
        }

        auto disassembled = std::get<LineTokens>(disRes);

        qDebug() << QString::number(iter.second, 2) << " = " << disassembled;
    }
}

QTEST_APPLESS_MAIN(tst_Assembler)
#include "tst_assembler.moc"
