#pragma once

#include <QByteArray>
#include <QHash>
#include <QMap>
#include <QMetaType>
#include <QString>
#include <optional>
#include <set>
#include <vector>

#include "ripes_types.h"

namespace Ripes {

enum SourceType {
    /** Assembly text */
    Assembly,
    /** C text */
    C,
    /** Flat binary external file */
    FlatBinary,
    /** Executable files not compiled from within ripes */
    ExternalELF,
    /** Executable files compiled within ripes */
    InternalELF
};

#define TEXT_SECTION_NAME ".text"

struct Symbol {
public:
    enum Type { Address = 1 << 0, Constant = 1 << 1 };
    Symbol(){};
    Symbol(const char* str) : v(str) {}
    Symbol(const QString& str) : v(str) {}
    Symbol(const QString& str, const Type _type) : v(str), type(_type) {}

    bool operator==(const Symbol& rhs) const { return this->v == rhs.v; }
    bool operator<(const Symbol& rhs) const { return this->v < rhs.v; }

    bool is(const Type t) const { return type & t; }
    bool is(const unsigned t) const { return type & t; }

    operator const QString&() const { return v; }

    QString v;
    unsigned type = 0;
};

using ReverseSymbolMap = std::map<AInt, Symbol>;

struct LoadFileParams {
    QString filepath;
    SourceType type;
    AInt binaryEntryPoint;
    AInt binaryLoadAt;
};

struct ProgramSection {
    QString name;
    AInt address;
    QByteArray data;
};

class DisassembledProgram {
public:
    /// Associates the given address and index with the disassembled instruction string
    void set(unsigned idx, VInt address, const QString& disres);

    /// Returns the disassembled instruction for the given index.
    std::optional<QString> getFromIdx(unsigned idx) const;

    /// Returns the disassembled instruction for the given address.
    std::optional<QString> getFromAddr(VInt address) const;

    std::optional<VInt> indexToAddress(unsigned idx) const;
    std::optional<unsigned> addressToIndex(VInt addr) const;

    /// Clears the disassembled program.
    void clear();

    /// Returns true if no disassembled program has been set.
    bool empty() const;

    unsigned numInstructions() const { return indexToAddressMap.size(); }

private:
    /// An ordered map of [instruction address : disassembled instruction]
    std::map<VInt, QString> addressToDisresMap;

    /// Ordered maps of the index of a given disassembled instruction to its address.
    std::map<unsigned, VInt> indexToAddressMap;
    std::map<VInt, unsigned> addressToIndexMap;
};

/**
 * @brief The Program struct
 * Wrapper around a program to be loaded into simulator memory. Text section shall contain the instructions of the
 * program. Others section may contain all other program sections (.bss, .data, ...)
 */
class Program {
public:
    // A source mapping is a mapping from {instruction address : source code lines}
    using SourceMapping = std::map<VInt, std::set<unsigned>>;

    AInt entryPoint = 0;
    std::map<QString, ProgramSection> sections;
    ReverseSymbolMap symbols;
    SourceMapping sourceMapping;

    // Hash of the source code which this program resulted from. Expected to be a SHA-1 hash (fastest).
    QString sourceHash;
    // Returns true if data is equal to the sourceHash of this program.
    bool isSameSource(const QByteArray& data) const;

    /// Returns the program section corresponding to the provided name. Return nullptr if no section was found with the
    /// given name.
    const ProgramSection* getSection(const QString& name) const;

    /// Returns the disassembled version of this program. The result is an ordered map of
    /// [instruction address : disassembled instruction]
    const DisassembledProgram& getDisassembled() const;
    const SourceMapping& getSourceMapping() const;

    /// Calculates a hash used for source identification.
    static QString calculateHash(const QByteArray& data);

private:
    /// A caching of the disassembled version of this program.
    mutable DisassembledProgram disassembled;
};

}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::SourceType);
