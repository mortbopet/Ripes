#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>
#include "defines.h"

/*
    Class for:
    - Parser: Parsing input binary files into simulator memory
    - Generating textual representation of binary instructions
*/

using namespace std;
typedef std::function<std::vector<uint32_t>(uint32_t)> decode_functor;

class Parser {
public:
    static Parser* getParser() {
        static Parser parser;
        return &parser;
    }

    int getFileSize() { return m_fileSize; }
    QString genStringRepr(uint32_t instr, uint32_t address) const;
    void clear();

    // Const interfaces to intstruction decode lamdas
    std::vector<uint32_t> decodeUInstr(uint32_t instr) const { return m_decodeUInstr(instr); }
    std::vector<uint32_t> decodeJInstr(uint32_t instr) const { return m_decodeJInstr(instr); }
    std::vector<uint32_t> decodeIInstr(uint32_t instr) const { return m_decodeIInstr(instr); }
    std::vector<uint32_t> decodeSInstr(uint32_t instr) const { return m_decodeSInstr(instr); }
    std::vector<uint32_t> decodeRInstr(uint32_t instr) const { return m_decodeRInstr(instr); }
    std::vector<uint32_t> decodeBInstr(uint32_t instr) const { return m_decodeBInstr(instr); }

    QString getInstructionString(uint32_t address) const;

    const QString& loadFromByteArray(QByteArray arr, bool disassembled = true, uint32_t baseAddress = 0x0);
    void loadFromByteArrayIntoData(QByteArray arr);
    const QString& loadBinaryFile(QString fileName, bool disassembled = true);
    const QString& getBinaryRepr() { return m_binaryRepr; }
    const QString& getDisassembledRepr() { return m_disassembledRepr; }
    QString getStringAt(uint32_t address) const;
    QByteArray getFileByteArray() { return m_fileByteArray; }
    bool initBinaryFile(char* filename);
    void parseFile();

private:
    Parser();
    ~Parser();
    ifstream m_fileStream;
    istreambuf_iterator<char> m_fileIter;
    QByteArray m_fileByteArray;
    int m_fileSize;
    QString m_disassembledRepr;  // disassembled representation of the currently loaded binary file in the pipeline
    QString m_binaryRepr;

    QMap<int, uint32_t>
        m_assembledInputFileMap;  // Intermediate assembler stage used for setting labels post-translation

    // Instruction decode lambda functions; runtime generated
    decode_functor generateWordParser(std::vector<int> bitFields);
    decode_functor m_decodeUInstr;
    decode_functor m_decodeJInstr;
    decode_functor m_decodeIInstr;
    decode_functor m_decodeSInstr;
    decode_functor m_decodeRInstr;
    decode_functor m_decodeBInstr;

    // String generating functions
    QString generateBranchString(uint32_t instr) const;
    QString generateLuiString(uint32_t instr) const;
    QString generateAuipcString(uint32_t instr) const;
    QString generateJalString(uint32_t instr, uint32_t address) const;
    QString generateJalrString(uint32_t instr) const;
    QString generateLoadString(uint32_t instr) const;
    QString generateStoreString(uint32_t instr) const;
    QString generateOpImmString(uint32_t instr) const;
    QString generateOpInstrString(uint32_t instr) const;
    QString generateEcallString(uint32_t instr) const;
};

#endif  // PARSER_H
