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

QString stringifyProgram(std::weak_ptr<const Program> program,
                         std::function<QString(const std::vector<char>&, uint32_t index)> stringifier,
                         AddrOffsetMap& addrOffsetMap) {
    if (auto sp = program.lock()) {
        const auto* textSection = sp->getSection(TEXT_SECTION_NAME);
        if (!textSection)
            return QString();

        QString out;
        auto dataStream = QDataStream(textSection->data);
        std::vector<char> buffer;
        const unsigned stride = ProcessorHandler::currentISA()->instrBytes();
        const unsigned regBytes = ProcessorHandler::currentISA()->bytes();
        buffer.resize(stride);

        int infoOffsets = 0;

        for (AInt addr = textSection->address; addr < textSection->address + textSection->data.length();
             addr += stride) {
            dataStream.readRawData(buffer.data(), stride);

            // symbol label
            if (sp->symbols.count(addr)) {
                const auto& symbol = sp->symbols.at(addr);
                // We are adding non-instruction lines to the output string. Record the line number as well as the sum
                // of invalid lines up to the given point.
                incrementAddressOffsetMap(out, addrOffsetMap, infoOffsets);
                out += "\n";
                incrementAddressOffsetMap(out, addrOffsetMap, infoOffsets, symbol.v);
                out += QString::number(addr, 16).rightJustified(regBytes * 2, '0') + " <" + symbol.v + ">:\n";
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

QString objdump(const std::shared_ptr<const Program>& program, AddrOffsetMap& addrOffsetMap) {
    auto assembler = ProcessorHandler::getAssembler();
    const unsigned instrBytes = ProcessorHandler::currentISA()->instrBytes();
    return stringifyProgram(
        program,
        [&program, &assembler, instrBytes](const std::vector<char>& buffer, AInt address) {
            VInt instr = 0;
            for (unsigned i = 0; i < instrBytes; i++) {
                instr |= (buffer[i] & 0xFF) << (CHAR_BIT * i);
            }
            return assembler->disassemble(instr, program->symbols, address).first;
        },
        addrOffsetMap);
}

QString binobjdump(const std::shared_ptr<const Program>& program, AddrOffsetMap& addrOffsetMap) {
    return stringifyProgram(
        program,
        [](const std::vector<char>& buffer, AInt) {
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
