#pragma once

#include <QStringList>

namespace Ripes {

bool isExecutable(const QString& path, const QStringList& dummyArgs = {});

}  // namespace Ripes
