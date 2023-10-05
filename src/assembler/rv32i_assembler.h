#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

class RV32I_Assembler : public QObject, public Assembler {
  Q_OBJECT

public:
  RV32I_Assembler(const ISAInfo<ISA::RV32I> *isa);

private:
  std::tuple<InstrVec, PseudoInstrVec>
  initInstructions(const ISAInfo<ISA::RV32I> *isa) const;

protected:
  QChar commentDelimiter() const override { return '#'; }
};

} // namespace Assembler
} // namespace Ripes
