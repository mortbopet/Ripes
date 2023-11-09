#pragma once

#include "assembler.h"
#include "isa/rv_i_ext.h"
#include "isa/rvisainfo_common.h"

#include <QObject>
#include <functional>

namespace Ripes {
namespace Assembler {

class RV64I_Assembler : public QObject, public Assembler {
  Q_OBJECT

public:
  RV64I_Assembler(const ISAInfo<ISA::RV64I> *isa);

private:
  std::tuple<InstrVec, PseudoInstrVec>
  initInstructions(const ISAInfo<ISA::RV64I> *isa) const;

protected:
  QChar commentDelimiter() const override { return '#'; }
};

} // namespace Assembler
} // namespace Ripes
