#pragma once

#include <map>

#include <QStringList>

namespace Ripes {

enum class ISA;
class ISAInfoBase;

std::map<ISA,
         std::function<std::shared_ptr<const ISAInfoBase>(const QStringList &)>>
constructConstructors();

} // namespace Ripes
