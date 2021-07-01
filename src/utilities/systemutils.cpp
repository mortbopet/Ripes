#include "systemutils.h"

#include <QProcess>
namespace Ripes {
bool isExecutable(const QString& path, const QStringList& dummyArgs) {
    QProcess process;
    process.start(path, dummyArgs);
    process.waitForFinished();
    return (process.error() != QProcess::FailedToStart);
}

}  // namespace Ripes
