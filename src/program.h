#pragma once

#include <QByteArray>
#include <vector>

namespace Ripes {

enum class FileType { Assembly, FlatBinary, Executable };

/**
 * @brief The Program struct
 * Wrapper around a program to be loaded into simulator memory. Text section shall contain the instructions of the
 * program. Others section may contain all other program sections (.bss, .data, ...)
 */
struct Program {
    using Section = std::pair<uint32_t, QByteArray*>;
    Section text;
    std::vector<Section> others;
};
}  // namespace Ripes
