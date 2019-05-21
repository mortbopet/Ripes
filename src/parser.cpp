#include "parser.h"
#include "defines.h"
#include "pipeline.h"

#include <cassert>
#include <iostream>

#include "binutils.h"

Parser::Parser() {
    // generate word parser functors
    m_decodeRInstr = generateWordParser(vector<int>{5, 3, 5, 5, 7});  // from LSB to MSB
    m_decodeIInstr = generateWordParser(vector<int>{5, 3, 5, 12});
    m_decodeSInstr = generateWordParser(vector<int>{5, 3, 5, 5, 7});
    m_decodeBInstr = generateWordParser(vector<int>{1, 4, 3, 5, 5, 6, 1});
    m_decodeUInstr = generateWordParser(vector<int>{5, 20});
    m_decodeJInstr = generateWordParser(vector<int>{5, 8, 1, 10, 1});
}

bool Parser::initBinaryFile(char* filename) {
    // Open binary file
    const string fname = string(filename);
    m_fileStream = ifstream(fname.c_str(), ios::binary);
    if (!(m_fileStream.good())) {
        return true;
    }

    // Create filestream iterator
    m_fileIter = istreambuf_iterator<char>(m_fileStream);

    // get file size
    m_fileStream.seekg(0, ios::end);
    m_fileSize = m_fileStream.tellg();
    m_fileStream.clear();
    m_fileStream.seekg(0, ios::beg);
    return false;
}

Parser::~Parser() {}

void Parser::parseFile() {
    // Parse the file in 8-bit segments and write to memory map
    MainMemory memory;
    int pc = 0;
    while (m_fileIter != istreambuf_iterator<char>()) {
        memory.write(pc, *m_fileIter, 1);
        pc++;
        m_fileIter++;
    }
    Pipeline::getPipeline()->setBaselineMemory(memory);
}

QString Parser::getStringAt(uint32_t address) const {
    // Returns the null-terminated string starting at @address
    MainMemory* memPtr = Pipeline::getPipeline()->getRuntimeMemoryPtr();
    QByteArray string;
    char read;
    do {
        read = memPtr->read(address) & 0xff;
        string.append(read);
        address++;
    } while (read != '\0');
    return QString::fromUtf8(string);
}

void Parser::clear() {
    m_disassembledRepr.clear();
    m_binaryRepr.clear();

    // Reset the pipeline
    Pipeline::getPipeline()->reset();
}

const QString& Parser::loadBinaryFile(QString fileName, bool disassembled) {
    // Loads a binary file and converts it to a text string, as well as puts the binary information into the pipeline
    // memorys text segment

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        m_fileByteArray = file.readAll();
        loadFromByteArray(m_fileByteArray);
        file.close();
    }
    if (disassembled) {
        return m_disassembledRepr;
    } else {
        return m_binaryRepr;
    }
}

const QString& Parser::loadFromByteArray(QByteArray arr, bool disassembled, uint32_t baseAddress) {
    // Loads the input arr into the memory of the simulator
    // Baseaddress is default = 0 (text). Can be changed for inserting into ie. data memory

    QString output = "";
    MainMemory memory;
    auto length = arr.length();
    QDataStream in(&arr, QIODevice::ReadOnly);
    char buffer[4];
    uint32_t byteIndex = baseAddress;
    for (int i = 0; i < length; i += 4) {
        in.readRawData(buffer, 4);
        QString binaryRepString;
        for (char j : buffer) {
            binaryRepString.prepend(QString().setNum((uint8_t)j, 2).rightJustified(8, '0'));
            memory.write(byteIndex, j, 1);
            byteIndex++;
        }
        m_binaryRepr.append(binaryRepString).append('\n');
        uint32_t instr =
            (buffer[3] & 0xff) << 24 | (buffer[2] & 0xff) << 16 | (buffer[1] & 0xff) << 8 | (buffer[0] & 0xff);
        output.append(genStringRepr(instr, byteIndex - 4));
        output.append("\n");
    }
    // Remove trailing \n character
    output.truncate(output.lastIndexOf('\n'));

    // Set the initial memory state of the processor
    Pipeline::getPipeline()->setBaselineMemory(memory);

    // Update the pipeline
    Pipeline::getPipeline()->update();
    if (baseAddress == 0)
        m_disassembledRepr = output;

    if (disassembled) {
        return m_disassembledRepr;
    } else {
        return m_binaryRepr;
    }
}

void Parser::loadFromByteArrayIntoData(QByteArray arr) {
    // Loads a byte array into the data segment of the simulator
    // In this, we do no string convertion etc., which is usually done for generating the disassembled view of the
    // program
    auto memPtr = Pipeline::getPipeline()->getDataMemoryPtr();
    auto length = arr.length();
    QDataStream in(&arr, QIODevice::ReadOnly);
    char buffer[4];
    uint32_t byteIndex = DATASTART;
    for (int i = 0; i < length; i += 4) {
        in.readRawData(buffer, 4);
        for (char & j : buffer) {
            memPtr->insert({byteIndex, j});
            byteIndex++;
        }
    }
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

QString Parser::genStringRepr(uint32_t instr, uint32_t address) const {
    switch (instr & 0x7f) {
        case LUI:
            return generateLuiString(instr);
        case AUIPC:
            return generateAuipcString(instr);
        case JAL:
            return generateJalString(instr, address);
        case JALR:
            return generateJalrString(instr);
        case BRANCH:
            return generateBranchString(instr);
        case LOAD:
            return generateLoadString(instr);
        case STORE:
            return generateStoreString(instr);
        case OP_IMM:
            return generateOpImmString(instr);
        case OP:
            return generateOpInstrString(instr);
        case ECALL:
            return generateEcallString(instr);
        default:
            return QString("Invalid instruction");
            break;
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
    return QString("lui x%1 %2").arg(fields[1]).arg(fields[0]);
}

QString Parser::generateAuipcString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeUInstr(instr);
    return QString("auipc x%1 %2").arg(fields[1]).arg((uint32_t)(fields[0]));
}

QString Parser::generateJalString(uint32_t instr, uint32_t address) const {
    std::vector<uint32_t> fields = decodeJInstr(instr);
    auto target = signextend<int32_t, 21>(fields[0] << 20 | fields[1] << 1 | fields[2] << 11 | fields[3] << 12);
    target += (address);
    // Check for misaligned four-byte boundary
    return QString("jal x%1 %2").arg(fields[4]).arg(target);
}

QString Parser::getInstructionString(uint32_t address) const {
    MainMemory* memPtr = Pipeline::getPipeline()->getRuntimeMemoryPtr();
    // Note: If address is not found in memory map, a default constructed object
    // will be created, and read. in our case uint8_t() = 0
    uint32_t read = (memPtr->read(address) | (memPtr->read(address + 1) << 8) | (memPtr->read(address + 2) << 16) |
                     (memPtr->read(address + 3) << 24));
    return genStringRepr(read, address);
}
