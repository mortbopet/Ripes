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

void DisassembledProgram::clear() {
    addressToDisresMap.clear();
    indexToAddressMap.clear();
    addressToIndexMap.clear();
}

bool DisassembledProgram::empty() const {
    return addressToDisresMap.empty();
}

std::optional<VInt> DisassembledProgram::indexToAddress(unsigned idx) const {
    auto it = indexToAddressMap.find(idx);
    if (it != indexToAddressMap.end())
        return it->second;
    return std::nullopt;
}

std::optional<unsigned> DisassembledProgram::addressToIndex(VInt addr) const {
    auto it = addressToIndexMap.find(addr);
    if (it != addressToIndexMap.end())
        return it->second;
    return std::nullopt;
}

void DisassembledProgram::set(unsigned int idx, VInt address, const QString& disres) {
    assert(indexToAddressMap.count(idx) == 0);
    assert(addressToDisresMap.count(address) == 0);
    indexToAddressMap[idx] = address;
    addressToIndexMap[address] = idx;
    addressToDisresMap[address] = disres;
}

std::optional<QString> DisassembledProgram::getFromAddr(VInt address) const {
    auto it = addressToDisresMap.find(address);
    if (it != addressToDisresMap.end())
        return {it->second};
    return {};
}

std::optional<QString> DisassembledProgram::getFromIdx(unsigned idx) const {
    auto it = indexToAddressMap.find(idx);
    if (it != indexToAddressMap.end())
        return getFromAddr(it->second);
    return {};
}

const DisassembledProgram& Program::getDisassembled() const {
    const auto* textSection = getSection(TEXT_SECTION_NAME);
    if (!textSection || textSection->data.size() == 0) {
        disassembled.clear();
        return disassembled;
    }
    if (disassembled.empty()) {
        // Initialize caching
        const unsigned instrBytes = ProcessorHandler::currentISA()->instrBytes();
        const VInt textSectionBaseAddr = textSection->address;
        unsigned line = 0;
        for (AInt addr = 0; addr < (AInt)ProcessorHandler::getCurrentProgramSize();) {
            const VInt disassembleAddr = addr + textSectionBaseAddr;
            auto disRes = ProcessorHandler::getAssembler()->disassemble(
                ProcessorHandler::getMemory().readMem(disassembleAddr, instrBytes),
                ProcessorHandler::getProgram()->symbols, disassembleAddr);
            // todo(mortbopet): shouldn't we do something about the possibility of the disassembling returning an error?
            const VInt realAddr = textSectionBaseAddr + addr;
            disassembled.set(line, realAddr, disRes.repr);
            addr += disRes.bytesDisassembled;
            ++line;
        }
    }
    return disassembled;
}

}  // namespace Ripes
