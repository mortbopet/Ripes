#include "objdump.h"

#include <QDataStream>
#include "../processorhandler.h"

namespace Ripes {
namespace Assembler {

static void incrementAddressOffsetMap(const QString& text, AddrOffsetMap& map, int& offsets,
                                      const QString& symbol = QString()) {
    map[text.count('\n')] = {offsets++, symbol};
}

/**
 * Stringifer
 * A stringifier is a function that takes a vector of bytes (representing an instruction word) alongside a base
 * address, and returns a string representation of the data.
 */
using Stringifier = std::function<OpDisassembleResult(const std::vector<char>&, AInt)>;

QString stringifyProgram(std::weak_ptr<const Program> program, Stringifier stringifier, AddrOffsetMap& addrOffsetMap) {
    if (auto sp = program.lock()) {
        const auto* textSection = sp->getSection(TEXT_SECTION_NAME);
        if (!textSection)
            return QString();

        QString out;
        auto dataStream = QDataStream(textSection->data);
        std::vector<char> buffer;
        const unsigned regBytes = ProcessorHandler::currentISA()->bytes();
        const unsigned instrBytes = ProcessorHandler::currentISA()->instrBytes();

        int infoOffsets = 0;
        const QString indent = "    ";

        AInt addr = textSection->address;
        while (addr < textSection->address + textSection->data.length()) {
            /// Ensure that we always have sizeof(Instr_T) bytes in the buffer, even though some of these bytes may be
            /// invalid (due to reading past the end of stream). For the end-of-program edge,case, the while loop will
            /// exit before we start decoding invalid data.
            size_t curBufSize = buffer.size();
            if (curBufSize < sizeof(Instr_T)) {
                buffer.resize(sizeof(Instr_T));
                dataStream.readRawData(buffer.data() + curBufSize, sizeof(Instr_T) - curBufSize);
            }

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
            out += indent + QString::number(addr, 16) + ":" + indent + indent;

            // Stringified instruction
            auto disres = stringifier(buffer, addr);
            if (disres.err.has_value()) {
                // Error during disassembled; we'll just have to increment the address counter by the default
                // instruction size of the ISA. std::min with buffer size in the edge case that we're erroring on a
                // single instruction which is smaller than the default instruction width.
                addr += std::min(static_cast<unsigned>(buffer.size()), instrBytes);
            } else
                addr += disres.bytesDisassembled;
            assert(buffer.size() >= disres.bytesDisassembled);

            // Instruction word
            QString wordString;
            for (size_t i = 0; i < disres.bytesDisassembled; ++i) {
                auto it = buffer.begin();
                wordString.prepend(QString().setNum(static_cast<uint8_t>(*it), 16).rightJustified(2, '0'));
                buffer.erase(it);
            }

            out += wordString + indent + indent;

            // Pad if < default instruction width to align disassembled instruction with default instruction width
            // column
            int dBytes = instrBytes - disres.bytesDisassembled;
            while (dBytes > 0) {
                out += indent;
                dBytes -= indent.size() / 2;
            }

            out += disres.repr + "\n";
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
            for (unsigned i = 0; i < instrBytes; ++i) {
                instr |= (buffer[i] & 0xFF) << (CHAR_BIT * i);
            }
            return assembler->disassemble(instr, program->symbols, address);
        },
        addrOffsetMap);
}

QString binobjdump(const std::shared_ptr<const Program>& program, AddrOffsetMap& addrOffsetMap) {
    auto assembler = ProcessorHandler::getAssembler();
    const unsigned instrBytes = ProcessorHandler::currentISA()->instrBytes();
    return stringifyProgram(
        program,
        [&program, &assembler, instrBytes](const std::vector<char>& buffer, AInt address) {
            /// Use disassembler to determine # of bytes disassembled, and then emit the byte representation.
            VInt instr = 0;
            for (unsigned i = 0; i < instrBytes; ++i) {
                instr |= (buffer[i] & 0xFF) << (CHAR_BIT * i);
            }
            auto disRes = assembler->disassemble(instr, program->symbols, address);
            disRes.repr.clear();
            for (size_t i = 0; i < disRes.bytesDisassembled; ++i) {
                disRes.repr.prepend(QString().setNum(static_cast<uint8_t>(buffer[i]), 2).rightJustified(8, '0'));
            }
            return disRes;
        },
        addrOffsetMap);
}

}  // namespace Assembler
}  // namespace Ripes
