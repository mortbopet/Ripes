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
struct Program {
    AInt entryPoint = 0;
    std::map<QString, ProgramSection> sections;
    ReverseSymbolMap symbols;

    const ProgramSection* getSection(const QString& name) const {
        const auto secIter =
            std::find_if(sections.begin(), sections.end(), [=](const auto& section) { return section.first == name; });

        if (secIter == sections.end()) {
            return nullptr;
        }

        return &secIter->second;
    }
};

}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::SourceType);
