#pragma once

#include "assembler/program.h"
#include <QFile>

namespace Ripes {

QString loadFlatBinaryFile(Program &program, const QString &filepath,
                           unsigned long entryPoint, unsigned long loadAt);

bool loadElfFile(Program &program, QFile &file);

} // namespace Ripes
