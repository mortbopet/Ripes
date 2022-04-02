#include "programutilities.h"

namespace Ripes {

QString loadFlatBinaryFile(Program &program, const QString &filepath,
                           unsigned long entryPoint, unsigned long loadAt) {
  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly)) {
    return "Error: Could not open file " + file.fileName();
  }
  ProgramSection section;
  section.name = TEXT_SECTION_NAME;
  section.address = loadAt;
  section.data = file.readAll();

  program.sections[TEXT_SECTION_NAME] = section;
  program.entryPoint = entryPoint;
  return QString();
}

} // namespace Ripes
