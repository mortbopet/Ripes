#include "rv32i_assembler.h"
#include "gnudirectives.h"
#include "ripessettings.h"
#include "rv_c_ext.h"
#include "rv_m_ext.h"

#include <QByteArray>
#include <algorithm>

namespace Ripes {
namespace Assembler {

RV32I_Assembler::RV32I_Assembler(
    const std::shared_ptr<const ISAInfo<ISA::RV32I>> &isa)
    : Assembler(isa) {
  initialize(gnuDirectives());

  // Initialize segment pointers and monitor settings changes to segment
  // pointers
  connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_TEXTSTART),
          &SettingObserver::modified, this, [this](const QVariant &value) {
            setSegmentBase(".text", value.toULongLong());
          });
  RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_TEXTSTART)->trigger();
  connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_DATASTART),
          &SettingObserver::modified, this, [this](const QVariant &value) {
            setSegmentBase(".data", value.toULongLong());
          });
  RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_DATASTART)->trigger();
  connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_BSSSTART),
          &SettingObserver::modified, this, [this](const QVariant &value) {
            setSegmentBase(".bss", value.toULongLong());
          });
  RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_BSSSTART)->trigger();
}

} // namespace Assembler
} // namespace Ripes
