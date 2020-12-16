#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>
#include <vector>

namespace Ripes {

enum class SourceType {
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

struct LoadFileParams {
    QString filepath;
    SourceType type;
    unsigned long binaryEntryPoint;
    unsigned long binaryLoadAt;
};

struct ProgramSection {
    QString name;
    unsigned long address;
    QByteArray data;
};

/**
 * @brief The Program struct
 * Wrapper around a program to be loaded into simulator memory. Text section shall contain the instructions of the
 * program. Others section may contain all other program sections (.bss, .data, ...)
 */
struct Program {
    unsigned long entryPoint = 0;
    std::map<QString, ProgramSection> sections;
    std::map<uint32_t, QString> symbols;

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
