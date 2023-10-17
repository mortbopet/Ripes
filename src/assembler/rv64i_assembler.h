#pragma once

#include "assembler.h"
#include "isa/rvisainfo_common.h"

#include <QObject>
#include <functional>

namespace Ripes {
namespace Assembler {

class RV64I_Assembler : public QObject, public Assembler<RVISA::RV64I> {
  Q_OBJECT

  /**
   * Extension enablers
   * Calling an extension enabler will register the appropriate assemblers and
   * pseudo-op expander functors with the assembler.
   */
  static void setExtension(RVISA::Extension ext, bool enabled);

protected:
  QChar commentDelimiter() const override { return '#'; }
};

} // namespace Assembler
} // namespace Ripes
