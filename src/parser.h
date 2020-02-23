#pragma once

#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>

#include "defines.h"
#include "program.h"

namespace Ripes {

/*
    Class for:
    - Parser: Parsing input binary files into simulator memory
    - Generating textual representation of binary instructions
*/

using namespace std;
typedef std::function<std::vector<uint32_t>(uint32_t)> decode_functor;

using AddrOffsetMap = std::map<unsigned long, int>;

class Parser {
public:
    static Parser* getParser() {
        static Parser parser;
        return &parser;
    }

    QString disassemble(const Program& program, uint32_t instr, uint32_t address) const;

    // Const interfaces to intstruction decode lamdas
    std::vector<uint32_t> decodeUInstr(uint32_t instr) const { return m_decodeUInstr(instr); }
    std::vector<uint32_t> decodeJInstr(uint32_t instr) const { return m_decodeJInstr(instr); }
    std::vector<uint32_t> decodeIInstr(uint32_t instr) const { return m_decodeIInstr(instr); }
    std::vector<uint32_t> decodeSInstr(uint32_t instr) const { return m_decodeSInstr(instr); }
    std::vector<uint32_t> decodeRInstr(uint32_t instr) const { return m_decodeRInstr(instr); }
    std::vector<uint32_t> decodeBInstr(uint32_t instr) const { return m_decodeBInstr(instr); }

    QString disassemble(const Program& program, AddrOffsetMap& addrOffsetMap) const;
    QString binarize(const Program& program, AddrOffsetMap& addrOffsetMap) const;

private:
    QString stringifyProgram(const Program& program, unsigned stride,
                             std::function<QString(const std::vector<char>& buffer, uint32_t index)> stringifier,
                             AddrOffsetMap& addrOffsetMap) const;

    Parser();
    ~Parser();

    // Instruction decode lambda functions; runtime generated
    decode_functor generateWordParser(std::vector<int> bitFields);
    decode_functor m_decodeUInstr;
    decode_functor m_decodeJInstr;
    decode_functor m_decodeIInstr;
    decode_functor m_decodeSInstr;
    decode_functor m_decodeRInstr;
    decode_functor m_decodeBInstr;

    // String generating functions
    QString generateBranchString(uint32_t instr, uint32_t address, const Program& program) const;
    QString generateLuiString(uint32_t instr) const;
    QString generateAuipcString(uint32_t instr) const;
    QString generateJalString(uint32_t instr, uint32_t address, const Program& program) const;
    QString generateJalrString(uint32_t instr) const;
    QString generateLoadString(uint32_t instr) const;
    QString generateStoreString(uint32_t instr) const;
    QString generateOpImmString(uint32_t instr) const;
    QString generateOpInstrString(uint32_t instr) const;
    QString generateEcallString(uint32_t instr) const;
};
}  // namespace Ripes
