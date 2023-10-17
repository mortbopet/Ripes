#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

class RV32I_Assembler : public QObject, public Assembler<RVISA::RV32I> {
  Q_OBJECT

public:
  RV32I_Assembler();

protected:
  QChar commentDelimiter() const override { return '#'; }
};

} // namespace Assembler
} // namespace Ripes
