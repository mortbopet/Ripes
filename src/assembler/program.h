#pragma once

#include <QByteArray>
#include <QMap>
#include <QMetaType>
#include <QString>
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

/**
 * @brief The Program struct
 * Wrapper around a program to be loaded into simulator memory. Text section shall contain the instructions of the
 * program. Others section may contain all other program sections (.bss, .data, ...)
 */
class Program {
public:
    AInt entryPoint = 0;
    std::map<QString, ProgramSection> sections;
    ReverseSymbolMap symbols;

    /// Returns the program section corresponding to the provided name. Return nullptr if no section was found with the
    /// given name.
    const ProgramSection* getSection(const QString& name) const;

    /// Returns the disassembled version of this program. The result is an ordered map of
    /// [instruction address : disassembled instruction]
    const std::map<VInt, QString>& getDisassembled() const;

private:
    /// A caching of the disassembled version of this program. An ordered map of
    /// [instruction address : disassembled instruction] with the iterator index equating to the line # of a given
    /// instruction if the disassembly is printed.
    mutable std::map<VInt, QString> disassembled;
};

}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::SourceType);
