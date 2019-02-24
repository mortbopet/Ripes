#include "assembler.h"
#include "binutils.h"
#include "defines.h"
#include "lexerutilities.h"

#include <QRegularExpression>
#include <QTextBlock>

namespace {
// Instruction groupings needed for various identification operations
const QStringList pseudoOps = QStringList() << "nop"
                                            << "la"
                                            << "nop"
                                            << "li"
                                            << "mv"
                                            << "not"
                                            << "neg"
                                            << "seqz"
                                            << "snez"
                                            << "sltz"
                                            << "sgtz"
                                            << "beqz"
                                            << "bgez"
                                            << "bnez"
                                            << "blez"
                                            << "bltz"
                                            << "bgtz"
                                            << "bgt"
                                            << "ble"
                                            << "bgtu"
                                            << "bleu"
                                            << "j"
                                            << "jal"
                                            << "jr"
                                            << "jalr"
                                            << "ret"
                                            << "call"
                                            << "tail"
                                            << "lb"
                                            << "lh"
                                            << "lw"
                                            << "sb"
                                            << "sh"
                                            << "sw";

const QStringList opsWithOffsets = QStringList() << "beq"
                                                 << "bne"
                                                 << "bge"
                                                 << "blt"
                                                 << "bltu"
                                                 << "bgeu"
                                                 << "jal"
                                                 << "auipc"
                                                 << "jalr";

// Grouping of instructions that require the same format
const QStringList opImmInstructions = QStringList() << "addi"
                                                    << "slli"
                                                    << "slti"
                                                    << "xori"
                                                    << "sltiu"
                                                    << "srli"
                                                    << "srai"
                                                    << "ori"
                                                    << "andi";

const QStringList opInstructions = QStringList() << "add"
                                                 << "sub"
                                                 << "mul"
                                                 << "mulh"
                                                 << "sll"
                                                 << "mulhsu"
                                                 << "slt"
                                                 << "mulhu"
                                                 << "sltu"
                                                 << "div"
                                                 << "xor"
                                                 << "srl"
                                                 << "sra"
                                                 << "divu"
                                                 << "rem"
                                                 << "or"
                                                 << "remu"
                                                 << "and";

const QStringList storeInstructions = QStringList() << "sb"
                                                    << "sh"
                                                    << "sw";

const QStringList loadInstructions = QStringList() << "lb"
                                                   << "lh"
                                                   << "lw"
                                                   << "lbu"
                                                   << "lhu";

const QStringList branchInstructions = QStringList() << "beq"
                                                     << "bne"
                                                     << "blt"
                                                     << "bge"
                                                     << "bltu"
                                                     << "bgeu";
const QStringList DataAssemblerDirectives = QStringList() << ".word"
                                                          << ".half"
                                                          << ".short"
                                                          << ".byte"
                                                          << ".2byte"
                                                          << ".4byte"
                                                          << ".long";
const static QMap<QString, size_t> DataAssemblerSizes{{".word", 4},  {".half", 2},  {".short", 2}, {".byte", 1},
                                                      {".2byte", 2}, {".4byte", 4}, {".long", 4}};
}  // namespace

Assembler::Assembler() {}

uint32_t Assembler::getRegisterNumber(const QString& reg) {
    // Converts a textual representation of a register to its numeric value
    QString regRes = reg;
    if (reg[0] == 'x') {
        regRes.remove('x');
        return regRes.toInt(nullptr, 10);
    } else {
        Q_ASSERT(ABInames.contains(reg));
        return ABInames[reg];
    }
}

int Assembler::getImmediate(QString string, bool& canConvert) {
    // Extracts an immediate number from a string, being base 10, 16 or 2
    canConvert = false;
    int immediate = string.toInt(&canConvert, 10);
    int sign = 1;
    if (!canConvert) {
        // Could not convert directly to integer - try hex or bin. Here, extra care is taken to account for a
        // potential sign, and include this is the range validation
        if (string[0] == '-' || string[0] == '+') {
            sign = string[0] == '-' ? -1 : 1;
            string.remove(0, 1);
        }
        if (string.startsWith(QLatin1String("0x"))) {
            immediate = string.remove("0x").toUInt(&canConvert, 16);
        } else if (string.startsWith(QLatin1String("0b"))) {
            immediate = string.remove("0b").toUInt(&canConvert, 2);
        } else {
            canConvert = false;
        }
    }
    return sign * immediate;
}

QByteArray Assembler::assembleOpImmInstruction(const QStringList& fields, int row) {
    Q_UNUSED(row);
    uint32_t funct3 = 0;
    bool hasFunct7 = false;
    uint32_t funct7 = 0;
    bool canConvert;
    int imm = getImmediate(fields[3], canConvert);
    if (fields[0] == "addi") {
        // Requires assembler-level support for labels (for pseudo-op 'la')
        if (canConvert) {
            // An immediate value as been provided
        } else {
            // An offset value has been provided ( unfolded pseudo-op)
            m_error |= !m_labelPosMap.contains(fields[3]);
            // calculate offset 31:12 bits - we -1 to get the row of the previois auipc op
            imm = m_labelPosMap[fields[3]] - (row - 1) * 4;
        }
        funct3 = 0b000;
    } else if (fields[0] == "slli") {
        funct3 = 0b001;
    } else if (fields[0] == "slti") {
        funct3 = 0b010;
    } else if (fields[0] == "xori") {
        funct3 = 0b100;
    } else if (fields[0] == "sltiu") {
        funct3 = 0b011;
    } else if (fields[0] == "srli") {
        funct3 = 0b101;
    } else if (fields[0] == "srai") {
        hasFunct7 = true;
        funct3 = 0b101;
        funct7 = 0b0100000;
    } else if (fields[0] == "ori") {
        funct3 = 0b110;
    } else if (fields[0] == "andi") {
        funct3 = 0b111;
    } else {
        m_error = true;
        Q_ASSERT(false);
    };

    if (hasFunct7) {
        return uintToByteArr(OP_IMM | funct3 << 12 | getRegisterNumber(fields[1]) << 7 |
                             getRegisterNumber(fields[2]) << 15 | funct7 << 25 | imm << 20);
    } else {
        return uintToByteArr(OP_IMM | funct3 << 12 | getRegisterNumber(fields[1]) << 7 |
                             getRegisterNumber(fields[2]) << 15 | imm << 20);
    }
}

QByteArray Assembler::assembleOpInstruction(const QStringList& fields, int row) {
    Q_UNUSED(row);
    uint32_t funct3 = 0;
    uint32_t funct7 = 0;
    if (fields[0] == "add") {
        funct3 = 0b000;
    } else if (fields[0] == "sub") {
        funct3 = 0b000;
        funct7 = 0b0100000;
    } else if (fields[0] == "mul") {
        funct3 = 0b000;
        funct7 = 0b0000001;
    } else if (fields[0] == "mulh") {
        funct3 = 0b001;
        funct7 = 0b0000001;
    } else if (fields[0] == "sll") {
        funct3 = 0b001;
    } else if (fields[0] == "mulhsu") {
        funct3 = 0b010;
        funct7 = 0b0000001;
    } else if (fields[0] == "slt") {
        funct3 = 0b010;
    } else if (fields[0] == "mulhu") {
        funct3 = 0b011;
        funct7 = 0b0000001;
    } else if (fields[0] == "sltu") {
        funct3 = 0b011;
    } else if (fields[0] == "div") {
        funct3 = 0b100;
        funct7 = 0b0000001;
    } else if (fields[0] == "xor") {
        funct3 = 0b100;
    } else if (fields[0] == "srl") {
        funct3 = 0b101;
    } else if (fields[0] == "sra") {
        funct3 = 0b101;
        funct7 = 0b0100000;
    } else if (fields[0] == "divu") {
        funct3 = 0b101;
        funct7 = 0b0000001;
    } else if (fields[0] == "rem") {
        funct3 = 0b110;
        funct7 = 0b0000001;
    } else if (fields[0] == "or") {
        funct3 = 0b110;
    } else if (fields[0] == "remu") {
        funct3 = 0b111;
        funct7 = 0b0000001;
    } else if (fields[0] == "and") {
        funct3 = 0b111;
    } else {
        m_error = true;
        Q_ASSERT(false);
    };
    return uintToByteArr(OP | funct3 << 12 | funct7 << 25 | getRegisterNumber(fields[1]) << 7 |
                         getRegisterNumber(fields[2]) << 15 | getRegisterNumber(fields[3]) << 20);
}

QByteArray Assembler::assembleStoreInstruction(const QStringList& fields, int row) {
    Q_UNUSED(row);
    uint32_t funct3 = 0;
    if (fields[0] == "sb") {
        funct3 = 0b000;
    } else if (fields[0] == "sh") {
        funct3 = 0b001;
    } else if (fields[0] == "sw") {
        funct3 = 0b010;
    } else {
        m_error = true;
        Q_ASSERT(false);
    };
    bool canConvert;
    int imm = getImmediate(fields[2], canConvert);
    if (canConvert) {
        // An offset value has been provided ( unfolded pseudo-op)
    } else {
        // A label was provided
        m_error |= !m_labelPosMap.contains(fields[2]);
        // calculate offset 31:12 bits - we -1 to get the row of the previois auipc op
        imm = m_labelPosMap[fields[2]] - (row - 1) * 4;
    }

    return uintToByteArr(STORE | getRegisterNumber(fields[3]) << 15 | getRegisterNumber(fields[1]) << 20 |
                         funct3 << 12 | (imm & 0b11111) << 7 | (imm & 0xFE0) << 20);
}

QByteArray Assembler::assembleLoadInstruction(const QStringList& fields, int row) {
    uint32_t funct3 = 0;
    if (fields[0] == "lb") {
        funct3 = 0b000;
    } else if (fields[0] == "lh") {
        funct3 = 0b001;
    } else if (fields[0] == "lw") {
        funct3 = 0b010;
    } else if (fields[0] == "lbu") {
        funct3 = 0b100;
    } else if (fields[0] == "lhu") {
        funct3 = 0b101;
    } else {
        m_error = true;
        Q_ASSERT(false);
    };
    bool canConvert;
    int imm = getImmediate(fields[2], canConvert);
    if (canConvert) {
        // An offset value has been provided ( unfolded pseudo-op)
    } else {
        m_error |= !m_labelPosMap.contains(fields[2]);
        // calculate offset 31:12 bits - we -1 to get the row of the previois auipc op
        imm = (m_labelPosMap[fields[2]] & 0xfff) - (row - 1) * 4;
    }

    return uintToByteArr(LOAD | funct3 << 12 | getRegisterNumber(fields[1]) << 7 | imm << 20 |
                         getRegisterNumber(fields[3]) << 15);
}

QByteArray Assembler::assembleBranchInstruction(const QStringList& fields, int row) {
    // calculate offset
    Q_ASSERT(m_labelPosMap.contains(fields[3]));
    int offset = m_labelPosMap[fields[3]];
    offset = offset - row * 4;  // byte-wize addressing
    uint32_t funct3 = 0;
    if (fields[0] == "beq") {
        funct3 = 0b000;
    } else if (fields[0] == "bne") {
        funct3 = 0b001;
    } else if (fields[0] == "blt") {
        funct3 = 0b100;
    } else if (fields[0] == "bge") {
        funct3 = 0b101;
    } else if (fields[0] == "bltu") {
        funct3 = 0b110;
    } else if (fields[0] == "bgeu") {
        funct3 = 0b111;
    } else {
        m_error = true;
        Q_ASSERT(false);
    };

    return uintToByteArr(BRANCH | getRegisterNumber(fields[1]) << 15 | getRegisterNumber(fields[2]) << 20 |
                         (offset & 0b11110) << 7 | (offset & 0x800) >> 4 | (offset & 0x7E0) << 20 |
                         (offset & 0x1000) << 19 | funct3 << 12);
}

QByteArray Assembler::assembleAuipcInstruction(const QStringList& fields, int row) {
    bool canConvert;
    int imm = getImmediate(fields[2], canConvert) << 12;
    if (canConvert) {
        // An immediate value as been provided
    } else {
        // An offset value has been provided
        m_error |= !m_labelPosMap.contains(fields[2]);
        // calculate offset 31:12 bits - we add +1 to offset the sign if the offset is negative
        imm = m_labelPosMap[fields[2]];
        if (imm < 0) {
            imm >>= 12;
            imm += 1;
            imm <<= 12;
        }
    }

    return uintToByteArr(AUIPC | getRegisterNumber(fields[1]) << 7 | (imm & 0xfffff000));
}

QByteArray Assembler::assembleJalrInstruction(const QStringList& fields, int row) {
    bool canConvert;
    int imm = getImmediate(fields[3], canConvert);
    if (canConvert) {
        // An immediate value as been provided
    } else {
        // An offset value has been provided ( unfolded pseudo-op)
        m_error |= !m_labelPosMap.contains(fields[3]);
        // calculate offset 31:12 bits - we -1 to get the row of the previois auipc op
        imm = m_labelPosMap[fields[3]] - (row - 1) * 4;
    }

    return uintToByteArr(JALR | getRegisterNumber(fields[1]) << 7 | getRegisterNumber(fields[2]) << 15 |
                         (imm & 0xfff) << 20);
}

void Assembler::assembleInstruction(const QStringList& fields, int row) {
    // Translates a single assembly instruction into binary
    QString instruction = fields[0];
    if (opImmInstructions.contains(instruction)) {
        m_textSegment.append(assembleOpImmInstruction(fields, row));
    } else if (opInstructions.contains(instruction)) {
        m_textSegment.append(assembleOpInstruction(fields, row));
    } else if (storeInstructions.contains(instruction)) {
        m_textSegment.append(assembleStoreInstruction(fields, row));
    } else if (loadInstructions.contains(instruction)) {
        m_textSegment.append(assembleLoadInstruction(fields, row));
    } else if (branchInstructions.contains(instruction)) {
        m_textSegment.append(assembleBranchInstruction(fields, row));
    } else if (instruction == "jalr") {
        m_textSegment.append(assembleJalrInstruction(fields, row));
    } else if (instruction == "lui") {
        bool canConvert;
        m_textSegment.append(
            uintToByteArr(LUI | getRegisterNumber(fields[1]) << 7 | getImmediate(fields[2], canConvert) << 12));
        m_error |= !canConvert;
    } else if (instruction == "auipc") {
        m_textSegment.append(assembleAuipcInstruction(fields, row));
    } else if (instruction == "jal") {
        Q_ASSERT(m_labelPosMap.contains(fields[2]));
        int32_t imm = m_labelPosMap[fields[2]];
        imm = imm - row * 4;
        imm = (imm & 0x7fe) << 20 | (imm & 0x800) << 9 | (imm & 0xff000) | (imm & 0x100000) << 11;
        m_textSegment.append(uintToByteArr(JAL | getRegisterNumber(fields[1]) << 7 | imm));
    } else if (instruction == "ecall") {
        m_textSegment.append(uintToByteArr(ECALL));
    } else {
        //  Unknown instruction
        m_error = true;
        Q_ASSERT(false);
    }
}

void Assembler::unpackPseudoOp(const QStringList& fields, int& pos) {
    if (fields.first() == "la") {
        m_instructionsMap[pos] = QStringList() << "auipc" << fields[1] << fields[2];
        m_instructionsMap[pos + 1] = QStringList() << "addi" << fields[1] << fields[1] << fields[2];
        m_lineLabelUsageMap[pos] = fields[2];
        pos += 2;
    } else if (fields.first() == "nop") {
        m_instructionsMap[pos] = QStringList() << "addi"
                                               << "x0"
                                               << "x0"
                                               << "0";
        pos++;
    } else if (fields.first() == "li") {
        // Determine whether an ADDI or LUI instruction is sufficient, or if both LUI and ADDI is needed, by analysing
        // the immediate size
        bool canConvert;
        int immediate = getImmediate(fields[2], canConvert);

        // Generate offset required for discerning between positive and negative immediates

        if (isInt<12>(immediate)) {
            // immediate can be represented by 12 bits, ADDI is sufficient
            m_instructionsMap[pos] = QStringList() << "addi" << fields[1] << "x0" << QString::number(immediate);
            pos++;
        } else {
            const int lower12Signed = signextend<int32_t, 12>(immediate & 0xFFF);
            int signOffset = lower12Signed < 0 ? 1 : 0;

            m_instructionsMap[pos] = QStringList()
                                     << "lui" << fields[1]
                                     << QString::number((static_cast<uint32_t>(immediate) >> 12) + signOffset);
            pos++;
            if ((immediate & 0xFFF) != 0) {
                m_instructionsMap[pos] = QStringList()
                                         << "addi" << fields[1] << fields[1] << QString::number(lower12Signed);
                pos++;
            }
        }
    } else if (fields.first() == "mv") {
        m_instructionsMap[pos] = QStringList() << "addi" << fields[1] << fields[2] << "0";
        pos++;
    } else if (fields.first() == "not") {
        m_instructionsMap[pos] = QStringList() << "xori" << fields[1] << fields[2] << "-1";
        pos++;
    } else if (fields.first() == "neg") {
        m_instructionsMap[pos] = QStringList() << "sub" << fields[1] << "x0" << fields[2];
        pos++;
    } else if (fields.first() == "seqz") {
        m_instructionsMap[pos] = QStringList() << "sltiu" << fields[1] << fields[2] << "1";
        pos++;
    } else if (fields.first() == "snez") {
        m_instructionsMap[pos] = QStringList() << "sltu" << fields[1] << fields[2] << "1";
        pos++;
    } else if (fields.first() == "sltz") {
        m_instructionsMap[pos] = QStringList() << "slt" << fields[1] << fields[2] << "x0";
        pos++;
    } else if (fields.first() == "sgtz") {
        m_instructionsMap[pos] = QStringList() << "slt" << fields[1] << "x0" << fields[2];
        pos++;
    } else if (fields.first() == "beqz") {
        m_instructionsMap[pos] = QStringList() << "beq" << fields[1] << "x0" << fields[2];
        pos++;
    } else if (fields.first() == "bnez") {
        m_instructionsMap[pos] = QStringList() << "bne" << fields[1] << "x0" << fields[2];
        pos++;
    } else if (fields.first() == "blez") {
        m_instructionsMap[pos] = QStringList() << "bge"
                                               << "x0" << fields[1] << fields[2];
        pos++;
    } else if (fields.first() == "bgez") {
        m_instructionsMap[pos] = QStringList() << "bge" << fields[1] << "x0" << fields[2];
        pos++;
    } else if (fields.first() == "bltz") {
        m_instructionsMap[pos] = QStringList() << "blt" << fields[1] << "x0" << fields[2];
        pos++;
    } else if (fields.first() == "bgtz") {
        m_instructionsMap[pos] = QStringList() << "blt"
                                               << "x0" << fields[1] << fields[2];
        pos++;
    } else if (fields.first() == "bgt") {
        m_instructionsMap[pos] = QStringList() << "blt" << fields[2] << fields[1] << fields[3];
        pos++;
    } else if (fields.first() == "ble") {
        m_instructionsMap[pos] = QStringList() << "bge" << fields[2] << fields[1] << fields[3];
        pos++;
    } else if (fields.first() == "bgtu") {
        m_instructionsMap[pos] = QStringList() << "bltu" << fields[2] << fields[1] << fields[3];
        pos++;
    } else if (fields.first() == "bleu") {
        m_instructionsMap[pos] = QStringList() << "bgeu" << fields[2] << fields[1] << fields[3];
        pos++;
    } else if (fields.first() == "j") {
        m_instructionsMap[pos] = QStringList() << "jal"
                                               << "x0" << fields[1];
        m_lineLabelUsageMap[pos] = fields[1];
        pos++;
    } else if (fields.first() == "jal") {
        if (fields.length() == 3) {
            // Non-pseudo op JAL
            m_instructionsMap[pos] = fields;
            m_lineLabelUsageMap[pos] = fields[2];
        } else {
            // Pseudo op JAL
            m_instructionsMap[pos] = QStringList() << "jal"
                                                   << "x1" << fields[1];
            m_lineLabelUsageMap[pos] = fields[1];
        }
        pos++;
    } else if (fields.first() == "jr") {
        m_instructionsMap[pos] = QStringList() << "jalr"
                                               << "x0" << fields[1] << "0";
        pos++;
    } else if (fields.first() == "jalr") {
        if (fields.length() == 4) {
            // Non-pseudo op JALR
            m_instructionsMap[pos] = fields;
        } else {
            // Pseudo op JALR
            m_instructionsMap[pos] = QStringList() << "jalr"
                                                   << "x1" << fields[1] << "0";
        }
        pos++;
    } else if (fields.first() == "ret") {
        m_instructionsMap[pos] = QStringList() << "jalr"
                                               << "x0"
                                               << "x1"
                                               << "0";
        pos++;
    } else if (fields.first() == "call") {
        m_instructionsMap[pos] = QStringList() << "auipc"
                                               << "x6" << fields[1];
        m_instructionsMap[pos + 1] = QStringList() << "jalr"
                                                   << "x1"
                                                   << "x6" << fields[1];
        m_lineLabelUsageMap[pos] = fields[1];
        m_lineLabelUsageMap[pos + 1] = fields[1];
        pos += 2;
    } else if (fields.first() == "tail") {
        m_instructionsMap[pos] = QStringList() << "auipc"
                                               << "x6" << fields[1];
        m_instructionsMap[pos + 1] = QStringList() << "jalr"
                                                   << "x0"
                                                   << "x6" << fields[1];
        m_lineLabelUsageMap[pos] = fields[1];
        m_lineLabelUsageMap[pos + 1] = fields[1];

        pos += 2;
    } else if (fields.first() == "la") {
        m_instructionsMap[pos] = QStringList() << "auipc" << fields[1] << fields[2];
        m_instructionsMap[pos + 1] = QStringList() << "addi" << fields[1] << fields[1] << fields[2];
        m_lineLabelUsageMap[pos] = fields[1];
        m_lineLabelUsageMap[pos + 1] = fields[1];

        pos += 2;
    } else if (fields.first() == "lb" || fields.first() == "lh" || fields.first() == "lw") {
        if (fields.length() == 4) {
            // Non-pseudo op load
            // convert immediate value
            bool canConvert;
            int imm = getImmediate(fields[2], canConvert);
            m_instructionsMap[pos] = QStringList() << fields[0] << fields[1] << QString::number(imm) << fields[3];
            pos++;
        } else {
            // Pseudo op load
            m_instructionsMap[pos] = QStringList() << "auipc" << fields[1] << fields[2];
            m_instructionsMap[pos + 1] = QStringList() << fields.first() << fields[1] << fields[2] << fields[1];
            m_lineLabelUsageMap[pos] = fields[1];
            m_lineLabelUsageMap[pos + 1] = fields[1];
            pos += 2;
        }
    } else if (fields.first() == "sb" || fields.first() == "sh" || fields.first() == "sw") {
        // not a pseudo op if the immediate value can be converted
        bool canConvert;
        int imm = getImmediate(fields[2], canConvert);
        if (canConvert) {
            // Non-pseudo op store
            m_instructionsMap[pos] = fields;
            pos++;
        } else {
            // Pseudo op store
            m_instructionsMap[pos] = QStringList() << "auipc" << fields[3] << fields[2];
            m_instructionsMap[pos + 1] = QStringList()
                                         << fields.first() << fields[1] << QString::number(imm) << fields[3];
            m_lineLabelUsageMap[pos] = fields[1];
            m_lineLabelUsageMap[pos + 1] = fields[1];
            pos += 2;
        }
    } else {
        // Unknown pseudo op
        m_error = true;
        Q_ASSERT(false);
    }
}

void Assembler::assembleWords(const QStringList& fields, QByteArray& byteArray, size_t size) {
    Q_ASSERT(size >= 1 && size <= 4);
    bool ok;
    for (int i = 1; i < fields.size(); i++) {
        qlonglong val = getImmediate(fields[i], ok);
        for (size_t i = 0; i < size; i++) {
            byteArray.append(val & 0xff);
            val >>= 8;
        }
    }
}

// Allocates $size bytes in the static data segment
void Assembler::assembleZeroArray(QByteArray& byteArray, size_t size) {
    Q_ASSERT(size >= 1);
    for (int i = 0; i < size; i++) {
        byteArray.append(static_cast<char>(0x0));
    }
}

void Assembler::assembleAssemblerDirective(const QStringList& fields) {
    QByteArray byteArray;
    if (fields[0] == QString(".string") || fields[0] == QString(".asciz")) {
        QString string;
        // Merge input fields
        for (int i = 1; i < fields.length(); i++) {
            QString strarg = fields[i];
            strarg.replace("\\n", "\n");
            string += strarg;
        }
        string.remove('\"');
        string.append('\0');
        byteArray = string.toUtf8();
    } else if (DataAssemblerDirectives.contains(fields[0])) {
        assembleWords(fields, byteArray, DataAssemblerSizes.value(fields[0]));
    } else if (fields[0] == QString(".zero")) {
        bool canConvert;
        assembleZeroArray(byteArray, static_cast<size_t>(getImmediate(fields[1], canConvert)));
        m_error |= !canConvert;
    } else if (fields[0] == QString(".data")) {
        // Following instructions will be assembled into the data segment
        m_inDataSegment = true;
        return;
    } else if (fields[0] == QString(".text")) {
        // Following instructions will be assembled in to the text segment
        m_inDataSegment = false;
        return;
    }

    // Since we want aligned memory accesses, we pad the byte array to word-sized indexes (4-byte chunks)
    if (byteArray.length() % 4 != 0) {
        int padding = 4 - byteArray.length() % 4;
        for (int i = 0; i < padding; i++)
            byteArray.append('\0');
    }
    m_dataSegment.append(byteArray);

    // Set hasData flag to trigger data segment insertion into simulator memory
    m_hasData = true;
}

void Assembler::unpackOp(const QStringList& _fields, int& pos) {
    // unpackOp
    // All pseudo-instructions will be converted to their corresponding sequence of operations
    // All hex- and binary immediate values will be converted to integer values, suitable for the assembly stage
    QStringList fields = _fields;

    // Check for labels
    QString string = fields[0];
    if (string.contains(':')) {
        // Label detected
        QStringList splitFirst = string.split(':');
        string = splitFirst[0];  // get label

        // Update fields vector
        splitFirst.removeAll("");
        if (splitFirst.size() == 1) {
            fields.removeAt(0);
        } else {
            fields[0] = splitFirst[1];
        }

        // Update map entries at given block
        if (m_inDataSegment) {
            m_labelPosMap[string] =
                // Offset label by data segment position and length of the data segment
                m_dataSegment.length() + DATASTART;  // Divide by 4 since labelPosMap is word indexed
        } else {
            // The label is in the text segment. Label is defined without an offset
            m_labelPosMap[string] = pos * 4;
        }
        if (fields.isEmpty()) {
            return;
        }
    }

    // Unpack operations
    if (pseudoOps.contains(fields[0])) {
        // A pseudo-operation is detected - unpack using unpackPseudoOp
        unpackPseudoOp(fields, pos);
    } else {
        if (fields[0][0] == '.') {
            // Assembler directive detected - handle directive (ie. setting data segment) POS is NOT incremented
            assembleAssemblerDirective(fields);
            return;
        } else if (opsWithOffsets.contains(fields[0])) {
            m_lineLabelUsageMap[pos] =
                fields.last();  // All offset using instructions have their offset as the last field value
        }
        // Add instruction to map and increment line counter by 1
        m_instructionsMap[pos] = fields;
        pos++;
    }
}

inline QByteArray Assembler::uintToByteArr(uint32_t in) {
    QByteArray out;
    for (int i = 0; i < 4; i++) {
        out.append((in)&0xff);
        in >>= 8;
    }
    return out;
}

void Assembler::restart() {
    m_error = false;
    m_hasData = false;
    m_instructionsMap.clear();
    m_lineLabelUsageMap.clear();
    m_labelPosMap.clear();
    m_textSegment.clear();
    m_dataSegment.clear();
}

namespace {
inline QStringList splitColon(const QString& string) {
    QStringList out = string.split(':');
    for (int i = 0; i < out.length() - 1; i++) {
        out[i].append(':');
    }
    out.removeAll("");
    return out;
}
}  // namespace

const QByteArray& Assembler::assembleBinaryFile(const QTextDocument& doc) {
    // Called by codeEditor when syntax has been accepted, and the document should be assembled into binary
    // Because of the previously accepted syntax, !no! error handling will be done, to ensure a fast execution
    int line = 0;
    restart();

    QStringList fields;
    for (QTextBlock block = doc.begin(); block != doc.end(); block = block.next()) {
        if (!block.text().isEmpty()) {
            // Split input into fields
            fields = block.text().split(splitter);
            fields.removeAll("");
            fields = splitQuotes(fields);
            if (!fields.isEmpty()) {
                // Split label fields, and keep separator ':'
                if (fields[0].contains(':')) {
                    auto firstFields = splitColon(fields.takeAt(0));
                    fields = firstFields + fields;
                }

                // Remove comments from syntax evaluation
                const static auto commentRegEx = QRegularExpression("[#](.*)");
                int commentIndex = fields.indexOf(commentRegEx);
                if (commentIndex != -1) {
                    int index = fields.length();
                    while (index >= commentIndex) {
                        fields.removeAt(index);
                        index--;
                    }
                }

                /* UnpackOp will:
                 *  -unpack & convert pseudo operations into its required number of operations
                 * - Record label positioning
                 * - Reord position of instructions which use labels
                 * - add instructions to m_instructionsMap
                 */
                if (fields.length() > 0) {
                    unpackOp(fields, line);
                }
            }
        }
    }

    // Assemble instruction(s)
    // Since the keys (line numbers) are sorted, we iterate straight over the map when inserting into the output
    // bytearray
    for (auto item : m_instructionsMap.toStdMap()) {
        assembleInstruction(item.second, item.first);
    }

    return m_textSegment;
}
