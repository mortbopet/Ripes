#include "systemutils.h"

#include <QProcess>
namespace Ripes {
bool isExecutable(const QString &path, const QStringList &dummyArgs) {
#ifdef RIPES_WITH_QPROCESS
  QProcess process;
  process.start(path, dummyArgs);
  process.waitForFinished();
  return (process.error() != QProcess::FailedToStart);
#else
  return false;
#endif
}

} // namespace Ripes
