#pragma once

#include "assembler/program.h"
#include <QFile>

namespace Ripes {

/**
 * @brief loadFlatBinaryFile
 * Loads a flat binary file into the program's text section.
 * @param program The program object to load the binary file into.
 * @param filepath The string containing the path to the binary file.
 * @param entryPoint The entry point address of the program.
 * @param loadAt The address at which to load the binary file.
 * @return An error message if an error occurred during loading; otherwise, an
 * empty string.
 *
 */
QString loadFlatBinaryFile(Program &program, const QString &filepath,
                           unsigned long entryPoint, unsigned long loadAt);

/**
 * @brief loadElfFile
 * Loads an ELF file into the program object passed as parameter.
 * @param program The program object to load the ELF file into.
 * @param file The QFile object containing the ELF file.
 * @return True if the ELF file was loaded successfully; otherwise, false.
 */
bool loadElfFile(Program &program, QFile &file);

} // namespace Ripes
