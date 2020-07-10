#include "systemio.h"

namespace Ripes {
QString SystemIO::s_fileErrorString;

std::map<int, QString> SystemIO::FileIOData::fileNames;
std::map<int, unsigned> SystemIO::FileIOData::fileFlags;
std::map<int, QTextStream> SystemIO::FileIOData::streams;
std::map<int, QFile> SystemIO::FileIOData::files;
QByteArray SystemIO::FileIOData::s_stdinBuffer;
QMutex SystemIO::FileIOData::s_stdioMutex;
QWaitCondition SystemIO::FileIOData::s_stdinBufferEmpty;
bool SystemIO::s_abortSyscall = false;
}  // namespace Ripes
