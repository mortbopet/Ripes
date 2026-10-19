#include <QtTest/QTest>

#include "assembler/matcher.h"
#include "isa/isainfo.h"
#include "isa/rv32isainfo.h"

#include "assembler/assembler.h"

#include "processorhandler.h"

using namespace Ripes;
using namespace Assembler;

// Tests which contains instructions or assembler directives not yet supported
const auto s_excludedTests = {"f", "ldst", "move", "recoding",
                              /* fails on CI, unknown as of know */ "memory"};

bool skipTest(const QString &test) {
  for (const auto &t : s_excludedTests) {
    if (test.startsWith(t)) {
      return true;
    }
  }
  return false;
}

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
  void tst_weirdDirectives();
  void tst_edgeImmediates();
  void tst_benchmarkNew();
  void tst_invalidreg();
  void tst_expression();
  void tst_invalidLabel();
  void tst_directives();
  void tst_stringDirectives();
  void tst_riscv();
  void tst_relativeLabels();
  void tst_parentheses();
  void tst_storeSymbolOffset();
  void tst_memOperandSyntax();
  void tst_extraOperands();

private:
  /// Assembles @p program (newline-separated source lines) for RV32I with
  /// @p extensions.
  static AssembleResult assemble(const QString &program,
                                 const QStringList &extensions = {}) {
    auto isa = std::make_shared<ISAInfo<ISA::RV32I>>(extensions);
    auto assembler = ISA_Assembler<ISA::RV32I>(isa);
    return assembler.assemble(program.split('\n'));
  }

  /// Assembles @p program and returns the .text section, or std::nullopt if
  /// assembly failed.
  static std::optional<QByteArray>
  tryAssembleText(const QString &program, const QStringList &extensions = {}) {
    auto res = assemble(program, extensions);
    if (res.errors.size() != 0)
      return std::nullopt;
    return res.program.getSection(".text")->data;
  }

  /// Assembles @p program and returns the words of the .text section.
  std::vector<uint32_t> assembleText(const QString &program) {
    auto res = assemble(program);
    if (res.errors.size() != 0) {
      res.errors.print();
      QTest::qFail(
          ("Failed to assemble program:\n" + program).toStdString().c_str(),
          __FILE__, __LINE__);
      return {};
    }
    std::vector<uint32_t> words;
    const auto &text = res.program.getSection(".text")->data;
    for (int i = 0; i + 3 < text.size(); i += 4) {
      words.push_back(static_cast<uint8_t>(text.at(i)) |
                      (static_cast<uint8_t>(text.at(i + 1)) << 8) |
                      (static_cast<uint8_t>(text.at(i + 2)) << 16) |
                      (static_cast<uint32_t>(static_cast<uint8_t>(
                           text.at(i + 3)))
                       << 24));
    }
    return words;
  }

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
  /// Assembles @p program (newline-separated source lines) and verifies the
  /// outcome against @p expect. If @p expectData is provided, the .data
  /// section is compared against it.
  void testAssemble(const QString &program, Expect expect,
                    QByteArray expectData = {}) {
    QString err;
    auto res = assemble(program);
    if ((res.errors.size() != 0) ^ (expect == Expect::Fail)) {
      res.errors.print();
      QString failExpectString = (expect == Expect::Fail ? "fail" : "success");
      err += "Expected " + failExpectString + " on program: \n";
      err += program;
      QFAIL(err.toStdString().c_str());
    }
    if (expectData.size() > 0) {
      const auto *dataProg = res.program.getSection(".data");
      const auto &dataSegment = res.program.getSection(".data")->data;
      if (expectData.size() != dataSegment.size()) {
        err += "Expected data segment of size " +
               QString::number(expectData.size()) + "B but found " +
               QString::number(dataSegment.size()) + "B\n";
        QFAIL(err.toStdString().c_str());
      }
      for (int i = 0; i < dataSegment.size(); i++) {
        if (dataSegment.at(i) != expectData.at(i)) {
          err += "Discrepancy in data segment at byte " + QString::number(i) +
                 " (address 0x" + QString::number(dataProg->address + i, 16) +
                 ")\n";
          err += "expected 0x" + QString::number(expectData.at(i)) +
                 " but found 0x" + QString::number(dataSegment.at(i)) + "\n";
          QFAIL(err.toStdString().c_str());
        }
      }
    }
  }
};

struct RVTestTuple {
  ProcessorID id;
  QString testDir;
};

QByteArray toByteArray(int64_t v, size_t size) {
  QByteArray arr;
  for (size_t i = 0; i < size; i++) {
    arr.append(v & 0xFF);
    v >>= 8;
  }
  return arr;
}

void tst_Assembler::tst_riscv() {
  // Tests all of the available RISC-V assembly programs
  std::vector<RVTestTuple> testTuples = {
      {ProcessorID::RV32_SS, RISCV32_TEST_DIR},
      {ProcessorID::RV64_SS, RISCV64_TEST_DIR}};

  auto testFunct = [](const QString &filename) {
    auto f = QFile(filename);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    auto program = QString(f.readAll());
    auto res = ProcessorHandler::getAssembler()->assembleRaw(program);
    if (res.errors.size() != 0) {
      QString errstr = "Failed while assembling file" + filename + "\n";
      errstr += res.errors.toString();
      QFAIL(errstr.toStdString().c_str());
    }
  };

  for (const auto &tt : testTuples) {
    ProcessorHandler::selectProcessor(tt.id, {"M"});
    const auto dir = QDir(tt.testDir);
    const auto testFiles = dir.entryList({"*.s"});
    for (const auto &test : testFiles) {
      if (skipTest(test))
        continue; // skip float tests
      testFunct(tt.testDir + QDir::separator() + test);
    }
  }
}

void tst_Assembler::tst_directives() {
  QByteArray expectData;

  // String constants
  const QStringList assembleStrings = {
      R"_("foo")_",   R"_("bar")_", R"_("1*2+(3/foo)")_",
      R"_("foo(")_",  R"_("foo)")_", R"_("foo(.)")_",
      R"_(".text")_", R"_("nop")_", R"_("addi a0 a0 baz")_"};

  QString program = ".data\n";
  int i = 0;
  for (const auto &s : assembleStrings) {
    program += "s" + QString::number(i++) + ": .string " + s + "\n";
    QString scpy = s;
    scpy.remove('\"');
    expectData += scpy.toUtf8().append('\0');
  }
  testAssemble(program, Expect::Success, expectData);

  // word, half and byte constants
  expectData.clear();
  expectData.append(42);
  expectData.append(toByteArray(0, 3));
  expectData.append(42);
  expectData.append(toByteArray(0, 1));
  expectData.append(42);

  testAssemble(R"(
    .data
    cw: .word 42
    ch: .half 42
    cb: .byte 42
  )",
               Expect::Success, expectData);

  // Too large constants (direct)
  testAssemble(R"(
    .data
    .byte 0xfff
  )",
               Expect::Fail);

  testAssemble(R"(
    .data
    .half 0xfffff
  )",
               Expect::Fail);

  testAssemble(R"(
    .data
    .word 0xfffffffff
  )",
               Expect::Fail);

  testAssemble(R"(
    .data
    .dword 0xfffffffffffffffff
  )",
               Expect::Fail);

  // Too large constants (indirect)
  testAssemble(R"(
    .data
    .equ abc 0xfff
    .byte abc
  )",
               Expect::Fail);

  testAssemble(R"(
    .data
    .equ abc 0xfffff
    .half abc
  )",
               Expect::Fail);

  testAssemble(R"(
    .data
    .equ abc 0xfffffffff
    .word abc
  )",
               Expect::Fail);

  testAssemble(R"(
    .data
    .equ abc 0xfffffffffffffffff
    .dword abc
  )",
               Expect::Fail);

  // .align directive
  expectData.clear();
  expectData.append(42);
  expectData.append(QByteArray(1, 0xFE).repeated(3));
  expectData.append(43);
  testAssemble(R"(
    .data
    .byte 42
    .align (2 + 2) 0xFE
    .byte 43
  )",
               Expect::Success, expectData);
}

void tst_Assembler::tst_expression() {
  testAssemble(R"(
    .text
    lw x10 (123 + (4* 3))(x10)
  )",
               Expect::Success);
  testAssemble(R"(
    .data
    A: .word 1
    .text
    lw a0 A(+1) a0
  )",
               Expect::Fail);
}

void tst_Assembler::tst_invalidLabel() {
  testAssemble(R"(
    .text
    ABC+: lw x10 ABC+ x10
  )",
               Expect::Fail);
  testAssemble("a: lw a0 a+ a0", Expect::Fail);
  testAssemble("addi a0 a0 (a", Expect::Fail);
}

void tst_Assembler::tst_benchmarkNew() {
  auto isa = std::make_shared<ISAInfo<ISA::RV32I>>(QStringList());
  auto assembler = ISA_Assembler<ISA::RV32I>(isa);
  auto program = createProgram(1000);
  QBENCHMARK { assembler.assembleRaw(program); }
}

void tst_Assembler::tst_simpleprogram() {
  testAssemble(R"(
    .data
    B: .word 1, 2, 2
    C: .string "hello world!"
    .text
    addi a0 a0 123 # Hello world
    nop
  )",
               Expect::Success);
}

void tst_Assembler::tst_simpleWithBranch() {
  testAssemble(R"(
    B:nop
    sw x0, 24(sp) # tmp. res 2
    addi a0 a0 10
    addi a0 a0 -1
    beqz a0 B
  )",
               Expect::Success);
}

void tst_Assembler::tst_weirdImmediates() {
  testAssemble(R"(
    addi a0 a0 0q1234
    addi a0 a0 -abcd
    addi a0 a0 100000000
    addi a0 a0 4096  # too large
    addi a0 a0 2048  # too large
    addi a0 a0 -2049 # too large
    addi a0 a0 0xabcdabcdabcd
  )",
               Expect::Fail);
}

void tst_Assembler::tst_weirdDirectives() {
  testAssemble(R"(
    .text
    B: .a

    .c
    nop
  )",
               Expect::Fail);
  // Test that a directive which requires no arguments throws error
  testAssemble(".data foo", Expect::Fail);
}

void tst_Assembler::tst_invalidreg() {
  testAssemble("addi x36 x46 1", Expect::Fail);
}

void tst_Assembler::tst_edgeImmediates() {
  testAssemble(R"(
    addi a0 a0 2047
    addi a0 a0 -2048
    # Hex and binary immediates which fits in 12 bits.
    addi x2, x0, 0xAFF
    addi x2, x0, 0b101010101010
    # Negative values expressed in hex should be accepted if they're
    # expressed using the full 32-bit immediate space.
    addi x2, x0, 0xFFFFFFF1
  )",
               Expect::Success);
}

void tst_Assembler::tst_label() {
  testAssemble(R"(
    A:

    B: C:
    D: E: addi a0 a0 -1
  )",
               Expect::Success);
}

void tst_Assembler::tst_segment() {
  testAssemble(R"(
    .data
    nop
    .text
    L: .word 1, 2, 3 ,4
    nop
    .data
    nop
  )",
               Expect::Success);
}

void tst_Assembler::tst_labelWithPseudo() {
  testAssemble(R"(
    j end
    end:nop
  )",
               Expect::Success);
}

void tst_Assembler::tst_stringDirectives() {
  testAssemble(R"(A: .string "a b c")", Expect::Success);
  testAssemble(R"(A: .string "a : c")", Expect::Success);
  testAssemble(R"(A: .string "a \" c")", Expect::Success);
  testAssemble(R"(A: .string "a \" \" c")", Expect::Success);
  testAssemble(R"(A: .string "a \\" c")", Expect::Fail);
  testAssemble(R"(A: .string "a c" "a c")", Expect::Fail);
  testAssemble(R"(A: .string "a c\")", Expect::Fail);
  testAssemble(R"(A: .string "a c)", Expect::Fail);
  testAssemble(R"(A: .string a c ")", Expect::Fail);
  testAssemble(R"(A: .string ")", Expect::Fail);
  testAssemble(R"(A: .string a c\")", Expect::Fail);
  testAssemble(R"(A: .string """")", Expect::Fail);
  testAssemble(R"(A: .string "")", Expect::Success);
}

void tst_Assembler::tst_relativeLabels() {
  testAssemble("1f:  bne x0 a0 1f", Expect::Fail);
  testAssemble(R"(
    1:  bne x0 a0 1f
    1:  bne x0 a0 1b
  )",
               Expect::Success);

  testAssemble(R"(
    bne x0 a0 2f
    2:
    1:  bne x0 a0 1f
    1:  bne x0 a0 1b
    bne x0 a0 2b
  )",
               Expect::Success);
  testAssemble(R"(
    bne x0 a0 2f
    2:
  )",
               Expect::Fail);
}

void tst_Assembler::tst_matcher() {
  auto isa = std::make_shared<ISAInfo<ISA::RV32I>>(QStringList());
  auto assembler = ISA_Assembler<ISA::RV32I>(isa);
  assembler.getMatcher().print();

  std::vector<std::pair<QString, unsigned>> toMatch = {
      {"beq", 0b11111110000000000000111011100011},
      {"addi", 0b00000111101100010000000010010011},
      {"slti", 0b00000000000100010010000100010011},
      {"xori", 0b00000000000100010100000100010011},
      {"slli", 0b00000000000100010001000100010011},
      {"srai", 0b01000000000100010101000100010011},
      {"add", 0b00000000001000010000000100110011},
      {"sub", 0b01000000001000010000000100110011}};

  for (const auto &iter : toMatch) {
    auto match = assembler.getMatcher().matchInstruction(iter.second);
    if (auto *error = std::get_if<Error>(&match)) {
      QFAIL(error->toString().toStdString().c_str());
    }

    auto matchInstr = std::get<const InstructionBase *>(match);
    if (matchInstr->name() != iter.first) {
      QString error = "Incorrect instruction decoded; got '" +
                      matchInstr->name() + "' but expected '" + iter.first +
                      "'";
      QFAIL(error.toStdString().c_str());
    }

    auto disRes = matchInstr->disassemble(iter.second, 0, {});
    if (auto *error = std::get_if<Error>(&disRes)) {
      QFAIL(error->toString().toStdString().c_str());
    }

    auto disassembled = std::get<LineTokens>(disRes);

    qDebug() << QString::number(iter.second, 2) << " = " << disassembled;
  }
}

void tst_Assembler::tst_parentheses() {
  testAssemble("lw x0, 0(x1)", Expect::Success);
  testAssemble("lw x0, 0(x1", Expect::Fail);
  testAssemble("lw x0, 0(x1 # ignored )", Expect::Fail);
  testAssemble("#(valid parentheses match)", Expect::Success);
  testAssemble("#)nonmatching parentheses(", Expect::Success);
}

void tst_Assembler::tst_storeSymbolOffset() {
  // Regression test for #467: a store with a symbolic offset, i.e.
  // `sw rs2, SYM(rs1)`, must assemble to a single store instruction, and not
  // be mistaken for the `sw rs2, label, rt` pseudo-instruction.
  const QString program = R"(
    .equ SYMBOL_2, 8
    %1
    .equ LATE, 4
  )";
  const std::vector<std::pair<QString, uint32_t>> cases = {
      {"sw t6, 8(sp)", 0x01F12423},
      {"sw t6, SYMBOL_2(sp)", 0x01F12423},
      {"sw t6, SYMBOL_2 (sp)", 0x01F12423},
      {"sw t6, (SYMBOL_2)(sp)", 0x01F12423},
      {"sw t6, SYMBOL_2*2(sp)", 0x01F12823},
      {"sh t6, SYMBOL_2(sp)", 0x01F11423},
      {"sb t6, SYMBOL_2(sp)", 0x01F10423},
      {"sw t6, LATE(sp)", 0x01F12223},
  };
  for (const auto &[line, expected] : cases) {
    auto words = assembleText(program.arg(line));
    if (words.size() != 1 || words.at(0) != expected) {
      QString err = "'" + line + "' assembled to:";
      for (auto w : words)
        err += " 0x" + QString::number(w, 16);
      err += " (expected 0x" + QString::number(expected, 16) + ")";
      QFAIL(err.toStdString().c_str());
    }
  }

  // The pseudo-instruction form must still expand to auipc + store, using the
  // last register as the temporary.
  auto words = assembleText(R"(
    sw t6, target, t0
    target: nop
  )");
  QCOMPARE(words.size(), 3u);
  QCOMPARE(words.at(0) & 0xFFF, (5u << 7) | 0b0010111); // auipc t0, ...
  QCOMPARE(words.at(1) & 0x01FFF07F, (31u << 20) | (5u << 15) | (0b010 << 12) |
                                         0b0100011); // sw t6, ...(t0)
}

void tst_Assembler::tst_memOperandSyntax() {
  // The GNU `offset(base)` memory operand syntax must assemble identically to
  // Ripes' native operand order, also when the offset is omitted.
  const QStringList exts = {"C"};
  const std::vector<std::pair<QString, QString>> cases = {
      {"lw t0, (sp)", "lw t0, 0(sp)"},
      {"lb t0, (sp)", "lb t0, 0(sp)"},
      {"lw t0, 0(sp)", "lw t0, 0 sp"},
      {"sw t0, (sp)", "sw t0, 0(sp)"},
      {"sb t0, (sp)", "sb t0, 0(sp)"},
      {"jalr ra, 4(t0)", "jalr ra, t0, 4"},
      {"jalr ra, (t0)", "jalr ra, t0, 0"},
      {"jalr ra, t0", "jalr ra, t0, 0"},
      {"jalr (t0)", "jalr ra, t0, 0"},
      {"jalr t0", "jalr ra, t0, 0"},
      {"c.lw a0, 4(a1)", "c.lw a0, a1, 4"},
      {"c.lw a0, (a1)", "c.lw a0, a1, 0"},
      {"c.sw a0, 4(a1)", "c.sw a0, a1, 4"},
      {"c.lwsp a0, 4(sp)", "c.lwsp a0, 4"},
      {"c.lwsp a0, (sp)", "c.lwsp a0, 0"},
      {"c.swsp a0, 4(x2)", "c.swsp a0, 4"},
  };
  for (const auto &[gnu, native] : cases) {
    auto gnuText = tryAssembleText(gnu, exts);
    auto nativeText = tryAssembleText(native, exts);
    QVERIFY2(nativeText.has_value(),
             ("Failed to assemble '" + native + "'").toStdString().c_str());
    QVERIFY2(gnuText.has_value(),
             ("Failed to assemble '" + gnu + "'").toStdString().c_str());
    QVERIFY2(*gnuText == *nativeText,
             ("'" + gnu + "' != '" + native + "'").toStdString().c_str());
  }

  // The implicit base register of sp-relative instructions must be sp.
  QVERIFY(!tryAssembleText("c.lwsp a0, 4(a1)", exts).has_value());
  QVERIFY(!tryAssembleText("c.swsp a0, 4(a1)", exts).has_value());
}

void tst_Assembler::tst_extraOperands() {
  // Superfluous operands must be rejected rather than silently ignored.
  const QStringList exts = {"C"};
  for (const QString line :
       {"addi a0, zero, 5, garbage", "add a0, zero, zero, zero",
        "lui a0, 1, 2", "sw t0, 4(sp), t1", "lw t0, 4(sp), t1",
        "jalr ra, 4(t0), t1", "c.li a0, 5, garbage", "c.lwsp a0, 4, sp"}) {
    QVERIFY2(!tryAssembleText(line, exts).has_value(),
             ("'" + line + "' was accepted").toStdString().c_str());
  }
}

QTEST_APPLESS_MAIN(tst_Assembler)
#include "tst_assembler.moc"
