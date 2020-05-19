#include "systemio.h"

namespace Ripes {
QString SystemIO::s_fileErrorString;

std::map<int, QString> SystemIO::FileIOData::fileNames;
std::map<int, unsigned> SystemIO::FileIOData::fileFlags;
std::map<int, QTextStream> SystemIO::FileIOData::streams;
std::map<int, QFile> SystemIO::FileIOData::files;

}  // namespace Ripes
