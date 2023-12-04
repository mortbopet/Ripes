#pragma once

#include "assembler.h"

#include <QObject>
#include <functional>

namespace Ripes {
namespace Assembler {

class RV64I_Assembler : public QObject, public Assembler {
  Q_OBJECT

public:
  RV64I_Assembler(const std::shared_ptr<const ISAInfo<ISA::RV64I>> &isa);

protected:
  QChar commentDelimiter() const override { return '#'; }
};

} // namespace Assembler
} // namespace Ripes
