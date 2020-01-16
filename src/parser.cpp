#include "parser.h"
#include "defines.h"

#include <cassert>
#include <functional>
#include <iostream>

#include "binutils.h"

namespace Ripes {

Parser::Parser() {
    // generate word parser functors
    m_decodeRInstr = generateWordParser(vector<int>{5, 3, 5, 5, 7});  // from LSB to MSB
    m_decodeIInstr = generateWordParser(vector<int>{5, 3, 5, 12});
    m_decodeSInstr = generateWordParser(vector<int>{5, 3, 5, 5, 7});
    m_decodeBInstr = generateWordParser(vector<int>{1, 4, 3, 5, 5, 6, 1});
    m_decodeUInstr = generateWordParser(vector<int>{5, 20});
    m_decodeJInstr = generateWordParser(vector<int>{5, 8, 1, 10, 1});
}

Parser::~Parser() {}

QString Parser::disassemble(const QByteArray& text) const {
    return stringifyByteArray(text, 4, [this](const std::vector<char>& buffer, uint32_t index) {
        // Hardcoded for RV32 for now
        uint32_t instr = 0;
        for (int i = 0; i < 4; i++) {
            instr |= (buffer[i] & 0xFF) << (CHAR_BIT * i);
        }
        return disassemble(instr, index);
    });
}

QString Parser::binarize(const QByteArray& text) const {
    return stringifyByteArray(text, 4, [](const std::vector<char>& buffer, uint32_t) {
        QString binaryString;
        for (auto byte : buffer) {
            binaryString.prepend(QString().setNum(static_cast<uint8_t>(byte), 2).rightJustified(8, '0'));
        }
        return binaryString;
    });
}

QString Parser::stringifyByteArray(const QByteArray& data, unsigned stride,
                                   std::function<QString(const std::vector<char>&, uint32_t index)> stringifier) const {
    QString out;
    auto dataStream = QDataStream(data);
    std::vector<char> buffer;
    buffer.resize(stride);
    uint32_t byteIndex = 0;

    for (int i = 0; i < data.length(); i += stride) {
        dataStream.readRawData(buffer.data(), stride);
        out += stringifier(buffer, byteIndex);
        out += "\n";
        byteIndex += stride;
    }
    return out;
}

decode_functor Parser::generateWordParser(std::vector<int> bitFields) {
    // Generates functors that can decode a binary number based on the input
    // vector which is supplied upon generation

    // Assert that total bitField size is (32-7)=25-bit. Subtract 7 for op-code
    int tot = 0;
    for (const auto& field : bitFields) {
        tot += field;
    }
    assert(tot == 25 && "Requested word parsing format is not 32-bit in length");

    // Generate vector of <fieldsize,bitmask>
    std::vector<std::pair<uint32_t, uint32_t>> parseVector;

    // Generate bit masks and fill parse vector
    for (const auto& field : bitFields) {
        parseVector.emplace_back(field, generateBitmask(field));
    }

    // Create parse functor
    decode_functor wordParser = [=](uint32_t word) {
        word = word >> 7;  // remove OpCode
        std::vector<uint32_t> parsedWord;
        for (const auto& field : parseVector) {
            parsedWord.insert(parsedWord.begin(), word & field.second);
            word = word >> field.first;
        }
        return parsedWord;
    };

    return wordParser;
}

QString Parser::disassemble(uint32_t instr, uint32_t address) const {
    switch (instr & 0x7f) {
        case instrType::LUI:
            return generateLuiString(instr);
        case instrType::AUIPC:
            return generateAuipcString(instr);
        case instrType::JAL:
            return generateJalString(instr, address);
        case instrType::JALR:
            return generateJalrString(instr);
        case instrType::BRANCH:
            return generateBranchString(instr);
        case instrType::LOAD:
            return generateLoadString(instr);
        case instrType::STORE:
            return generateStoreString(instr);
        case instrType::OP_IMM:
            return generateOpImmString(instr);
        case instrType::OP:
            return generateOpInstrString(instr);
        case instrType::ECALL:
            return generateEcallString(instr);
        default:
            return QString("Invalid instruction");
    }
}
QString Parser::generateEcallString(uint32_t) const {
    return QString("ecall");
}

QString Parser::generateOpInstrString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeRInstr(instr);
    switch (fields[3]) {
        case 0b000:
            if (fields[0] == 0) {
                return QString("add x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            } else if (fields[0] == 0b0100000) {
                return QString("sub x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
                break;
            } else if (fields[0] == 0b0000001) {
                return QString("mul x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            }
        case 0b001:
            if (fields[0] == 0b0000001) {
                return QString("mulh x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            } else if (fields[0] == 0) {
                return QString("sll x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            }
        case 0b010:
            if (fields[0] == 0b0000001) {
                return QString("mulhsu x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            } else if (fields[0] == 0) {
                return QString("slt x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            }
        case 0b011:
            if (fields[0] == 0b0000001) {
                return QString("mulhu x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            } else {
                return QString("sltu x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            }
        case 0b100:
            if (fields[0] == 0b1) {
                return QString("div x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            } else {
                return QString("xor x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            }
        case 0b101:
            if (fields[0] == 0) {
                return QString("srl x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);

            } else if (fields[0] == 0b0100000) {
                return QString("sra x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);

            } else if (fields[0] == 0b1) {
                return QString("divu x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            }
        case 0b110:
            if (fields[0] == 0b1) {
                return QString("rem x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            } else {
                return QString("or x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            }
        case 0b111:
            if (fields[0] == 0b1) {
                return QString("remu x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            } else {
                return QString("and x%1 x%2 x%3").arg(fields[4]).arg(fields[2]).arg(fields[1]);
            }
        default:
            return QString("Unknown instruction");
    }
}

QString Parser::generateOpImmString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeIInstr(instr);
    switch (fields[2]) {
        case 0b000:  // ADDI
            if (fields[3] == 0 && fields[1] == 0) {
                // nop special case
                return QString("nop");
            }
            return QString("addi x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(signextend<int32_t, 12>(fields[0]));
        case 0b001:  // SLLI
            return QString("slli x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(fields[0] & 0b11111);
        case 0b010:  // SLTI
            return QString("slti x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(signextend<int32_t, 12>(fields[0]));
        case 0b011:  // SLTIU
            return QString("sltiu x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(fields[0]);
        case 0b100:  // XORI
            return QString("xori x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(fields[0]);
        case 0b101:
            if ((fields[0] >> 5) == 0) {
                return QString("srli x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(fields[0] & 0b11111);
            } else {  // SRAI
                return QString("srai x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(fields[0] & 0b11111);
            }
        case 0b110:  // ORI
            return QString("ori x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(fields[0]);
        case 0b111:  // ANDI
            return QString("andi x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(fields[0]);
        default:
            return QString();
    }
}

QString Parser::generateStoreString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeSInstr(instr);
    auto offset = signextend<int32_t, 12>((fields[0] << 5) | fields[4]);
    switch (fields[3]) {
        case 0b000:  // SB
            return QString("sb x%1 %2(x%3)").arg(fields[1]).arg(offset).arg(fields[2]);
        case 0b001:  // SH
            return QString("sh x%1 %2(x%3)").arg(fields[1]).arg(offset).arg(fields[2]);
        case 0b010:  // SW
            return QString("sw x%1 %2(x%3)").arg(fields[1]).arg(offset).arg(fields[2]);
        default:
            return QString();
    }
}

QString Parser::generateLoadString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeIInstr(instr);

    // Handle different load types by pointer casting and subsequent
    // dereferencing. This will handle whether to sign or zero extend.
    switch (fields[2]) {
        case 0b000:  // LB - load sign extended byte
            return QString("lb x%1 %2(x%3)").arg(fields[3]).arg(signextend<int, 12>(fields[0])).arg(fields[1]);
        case 0b001:  // LH load sign extended halfword
            return QString("lh x%1 %2(x%3)").arg(fields[3]).arg(signextend<int, 12>(fields[0])).arg(fields[1]);
        case 0b010:  // LW load word
            return QString("lw x%1 %2(x%3)").arg(fields[3]).arg(signextend<int, 12>(fields[0])).arg(fields[1]);
        case 0b100:  // LBU load zero extended byte
            return QString("lbu x%1 %2(x%3)").arg(fields[3]).arg(signextend<int, 12>(fields[0])).arg(fields[1]);
        case 0b101:  // LHU load zero extended halfword
            return QString("lhu x%1 %2(x%3)").arg(fields[3]).arg(signextend<int, 12>(fields[0])).arg(fields[1]);
        default:
            return QString();
    }
}

QString Parser::generateBranchString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeBInstr(instr);
    auto offset = signextend<int32_t, 13>((fields[0] << 12) | (fields[1] << 5) | (fields[5] << 1) | (fields[6] << 11));
    switch (fields[4]) {
        case 0b000:  // BEQ
            return QString("beq x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b001:  // BNE
            return QString("bne x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b100:  // BLT - signed comparison
            return QString("blt x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b101:  // BGE - signed comparison
            return QString("bge x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b110:  // BLTU
            return QString("bltu x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b111:  // BGEU
            return QString("bgeu x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        default:
            return QString();
    }
}

QString Parser::generateJalrString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeIInstr(instr);
    return QString("jalr x%1 x%2 %3").arg(fields[3]).arg(fields[1]).arg(signextend<int32_t, 12>(fields[0]));
}

QString Parser::generateLuiString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeUInstr(instr);
    return QString("lui x%1 %2").arg(fields[1]).arg("0x" + QString::number(fields[0]));
}

QString Parser::generateAuipcString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeUInstr(instr);
    return QString("auipc x%1 %2").arg(fields[1]).arg("0x" + QString::number(fields[0]));
}

QString Parser::generateJalString(uint32_t instr, uint32_t address) const {
    std::vector<uint32_t> fields = decodeJInstr(instr);
    auto target = signextend<int32_t, 21>(fields[0] << 20 | fields[1] << 1 | fields[2] << 11 | fields[3] << 12);
    target += (address);
    // Check for misaligned four-byte boundary
    return QString("jal x%1 %2").arg(fields[4]).arg("0x" + QString::number(target, 16));
}
}  // namespace Ripes
