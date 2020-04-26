#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>
#include <vector>

namespace Ripes {

enum class FileType { Assembly, C, FlatBinary, Executable };

#define TEXT_SECTION_NAME ".text"

struct LoadFileParams {
    QString filepath;
    FileType type;
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
    std::vector<ProgramSection> sections;
    std::map<unsigned long, QString> symbols;

    const ProgramSection* getSection(const QString& name) const {
        const auto secIter =
            std::find_if(sections.begin(), sections.end(), [=](const auto& section) { return section.name == name; });

        if (secIter == sections.end()) {
            return nullptr;
        }

        return &*secIter;
    }
};
}  // namespace Ripes
