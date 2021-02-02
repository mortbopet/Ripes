#include "objdump.h"

#include <QDataStream>
#include "../processorhandler.h"

namespace Ripes {
namespace Assembler {

namespace {
void incrementAddressOffsetMap(const QString& text, AddrOffsetMap& map, int& offsets,
                               const QString& symbol = QString()) {
    map[text.count('\n')] = {offsets++, symbol};
}
}  // namespace

QString stringifyProgram(std::weak_ptr<const Program> program, unsigned stride,
                         std::function<QString(const std::vector<char>&, uint32_t index)> stringifier,
                         AddrOffsetMap& addrOffsetMap) {
    if (auto sp = program.lock()) {
        const auto* textSection = sp->getSection(TEXT_SECTION_NAME);
        if (!textSection)
            return QString();

        QString out;
        auto dataStream = QDataStream(textSection->data);
        std::vector<char> buffer;
        buffer.resize(stride);

        int infoOffsets = 0;

        for (unsigned long addr = textSection->address; addr < textSection->address + textSection->data.length();
             addr += stride) {
            dataStream.readRawData(buffer.data(), stride);

            // symbol label
            if (sp->symbols.count(addr)) {
                const auto& symbol = sp->symbols.at(addr);
                // We are adding non-instruction lines to the output string. Record the line number as well as the sum
                // of invalid lines up to the given point.
                incrementAddressOffsetMap(out, addrOffsetMap, infoOffsets);
                out += "\n";
                incrementAddressOffsetMap(out, addrOffsetMap, infoOffsets, symbol);
                out += QString::number(addr, 16).rightJustified(8, '0') + " <" + symbol + ">:\n";
            }

            // Instruction address
            out += "\t" + QString::number(addr, 16) + ":\t\t";

            // Instruction word
            QString wordString;
            for (auto byte : buffer) {
                wordString.prepend(QString().setNum(static_cast<uint8_t>(byte), 16).rightJustified(2, '0'));
            }
            out += wordString + "\t\t";

            // Stringified instruction
            out += stringifier(buffer, addr) + "\n";
        }
        return out;
    }
    return QString();
}

QString objdump(std::shared_ptr<const Program> program, AddrOffsetMap& addrOffsetMap) {
    auto assembler = ProcessorHandler::get()->getAssembler();
    return stringifyProgram(
        program, 4,
        [&program, &assembler](const std::vector<char>& buffer, uint32_t address) {
            // Hardcoded for RV32 for now
            uint32_t instr = 0;
            for (int i = 0; i < 4; i++) {
                instr |= (buffer[i] & 0xFF) << (CHAR_BIT * i);
            }
            return assembler->disassemble(instr, program->symbols, address)
                .first;  // disassemble(program, instr, index);
        },
        addrOffsetMap);
}

QString binobjdump(std::shared_ptr<const Program> program, AddrOffsetMap& addrOffsetMap) {
    return stringifyProgram(
        program, 4,
        [](const std::vector<char>& buffer, uint32_t) {
            QString binaryString;
            for (auto byte : buffer) {
                binaryString.prepend(QString().setNum(static_cast<uint8_t>(byte), 2).rightJustified(8, '0'));
            }
            return binaryString;
        },
        addrOffsetMap);
}

}  // namespace Assembler
}  // namespace Ripes
