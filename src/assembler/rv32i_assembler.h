#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"

namespace Ripes {
namespace Assembler {

class RV32I_Assembler : public QObject, public Assembler {
  Q_OBJECT

public:
  RV32I_Assembler(const std::shared_ptr<const ISAInfo<ISA::RV32I>> &isa);

protected:
  QChar commentDelimiter() const override { return '#'; }
};

} // namespace Assembler
} // namespace Ripes
