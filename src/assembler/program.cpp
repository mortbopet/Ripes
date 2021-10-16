#include "program.h"

#include "processorhandler.h"

namespace Ripes {

const ProgramSection* Program::getSection(const QString& name) const {
    const auto secIter =
        std::find_if(sections.begin(), sections.end(), [=](const auto& section) { return section.first == name; });

    if (secIter == sections.end()) {
        return nullptr;
    }

    return &secIter->second;
}

const std::map<VInt, QString>& Program::getDisassembled() const {
    const auto* textSection = getSection(TEXT_SECTION_NAME);
    if (!textSection || textSection->data.size() == 0) {
        disassembled.clear();
        return disassembled;
    }
    if (disassembled.size() == 0) {
        // Initialize caching
        const unsigned instrBytes = ProcessorHandler::currentISA()->instrBytes();
        const VInt textSectionBaseAddr = textSection->address;
        for (AInt addr = 0; addr < (AInt)ProcessorHandler::getCurrentProgramSize();) {
            const VInt disassembleAddr = addr + textSectionBaseAddr;
            auto disRes = ProcessorHandler::getAssembler()->disassemble(
                ProcessorHandler::getMemory().readMem(disassembleAddr, instrBytes),
                ProcessorHandler::getProgram()->symbols, disassembleAddr);
            // todo(mortbopet): shouldn't we do something about the possibility of the disassembling returning an error?
            const VInt realAddr = textSectionBaseAddr + addr;
            disassembled[realAddr] = disRes.repr;
            addr += disRes.bytesDisassembled;
        }
    }
    return disassembled;
}

}  // namespace Ripes
