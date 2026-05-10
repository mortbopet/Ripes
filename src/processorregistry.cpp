#include "processorregistry.h"

#include <QPolygonF>

#include "processors/RISC-V/rv5mc/rv5mc_1m.h"
#include "processors/RISC-V/rv5mc/rv5mc_2m.h"
#include "processors/RISC-V/rv5s/rv5s.h"
#include "processors/RISC-V/rv5s_float_no_fw_hz/rv5s_float_no_fw_hz.h"
#include "processors/RISC-V/rv5s_no_fw/rv5s_no_fw.h"
#include "processors/RISC-V/rv5s_no_fw_hz/rv5s_no_fw_hz.h"
#include "processors/RISC-V/rv5s_no_hz/rv5s_no_hz.h"
#include "processors/RISC-V/rv6s_dual/rv6s_dual.h"
#include "processors/RISC-V/rvss/rvss.h"
#include "processors/RISC-V/rvss_float/rvss_float.h"

namespace Ripes {

// TODO: This whole registration logic is ugly and should be reworked - either
// load a JSON file at runtime, or have a small codegen pass during compilation
// to generate this file based on an input JSON.

//------------------------------------------------------------------------------
// Descriptions
//------------------------------------------------------------------------------

#define no_hz_note                                                             \
  "<br><b>NOTE: given the lack of a hazard unit (and "                         \
  "therefore the lack of dynamic hazard resolution in the pipeline) programs " \
  "with hazards will execute incorrectly, unless resolved by inserting "       \
  "nop's to manually stall the pipeline.</b>";

constexpr const char rv5s_no_fw_hz_desc[] =
    "A 5-stage in-order processor with no forwarding or hazard "
    "detection/elimination." no_hz_note;

constexpr const char rv5s_float_no_fw_hz_desc[] =
    "A 5-stage in-order processor with additional floating point unit but with "
    "no forwarding or hazard "
    "detection/elimination." no_hz_note;

constexpr const char rv5s_no_hz_desc[] =
    "A 5-stage in-order processor with forwarding but no hazard "
    "detection/elimination." no_hz_note;
constexpr const char rv5s_desc[] =
    "A 5-stage in-order processor with hazard detection/elimination and "
    "forwarding.";
constexpr const char rv5s_no_fw_desc[] =
    "A 5-stage in-order processor with hazard detection/elimination but no "
    "forwarding unit.";

constexpr const char rv6s_desc[] =
    "A 6-stage dual-issue in-order processor. Each way may execute "
    "arithmetic instructions, whereas way 1 "
    "is reserved for controlflow and ecall instructions, and way 2 for "
    "memory accessing instructions.";

constexpr const char rv5mc_desc[] =
    "A 5 stage multicycle processor similar to the multicycle processor "
    "described in Harris&Harris. Instructions take a variable number of "
    "cycles to execute. "
    "By default, the current state is shown in the control unit. "
    "This version uses separate memories for data and instructions like the "
    "single cycle or segmented processors.";

constexpr const char rv5mc_desc_1m[] =
    "A 5 stage multicycle processor similar to the multicycle processor "
    "described in Harris&Harris. Instructions take a variable number of "
    "cycles to execute. "
    "By default, the current state is shown in the control unit. "
    "This version takes advantage of the fact that data and instructions are "
    "never accessed in the same cycle to use a single memory for data and "
    "instructions.";

static const RegisterInitialization defaultRegVals = {
    {RVISA::GPR, {{2, 0x7ffffff0}, {3, 0x10000000}}}};

//------------------------------------------------------------------------------
// implementation
//------------------------------------------------------------------------------

template <template <typename> typename ProcType, typename ID_t>
static void
add32And64BitVariations(ProcClassInfo &procClassInfo, const char *varName,
                        const char *varDesc, const ID_t variationID32,
                        const ID_t variationID64,
                        const VariationInfo::Options_t &baseVariationOptions,
                        const std::vector<Layout> &layouts,
                        const RegisterInitialization &defaultRegVals) {
  procClassInfo.addVariation(
      std::make_unique<ProcVariationInfo<ProcType<uint32_t>>>(
          QString(varName), QString(varDesc),
          VariationInfo{variationID32, baseVariationOptions,
                        VariationInfo::BIT_WIDTH_32},
          layouts, defaultRegVals));
  procClassInfo.addVariation(
      std::make_unique<ProcVariationInfo<ProcType<uint64_t>>>(
          QString(varName), QString(varDesc),
          VariationInfo{variationID64, baseVariationOptions,
                        VariationInfo::BIT_WIDTH_64},
          layouts, defaultRegVals));
}

static ProcClassInfo register_rv_ss() {
  std::vector<Layout> layouts;

  ProcClassInfo rv_ss_info(ProcessorID::RV_SS, ISA::RV32I,
                           QStringLiteral("Single-cycle RISC-V"),
                           Variations::RV_SS::RV32I);

  // RISC-V Base single cycle
  layouts = {{"Standard",
              ":/layouts/RISC-V/rvss/rv_ss_standard_layout.json",
              {{{0, 0}, QPointF{0.5, 0}}}},
             {"Extended",
              ":/layouts/RISC-V/rvss/rv_ss_extended_layout.json",
              {{{0, 0}, QPointF{0.5, 0}}}}};
  add32And64BitVariations<vsrtl::core::RVSS>(
      rv_ss_info, "Single-cycle processor", "A single cycle processor",
      Variations::RV_SS::RV32I, Variations::RV_SS::RV64I, {}, layouts,
      defaultRegVals);

  // RISC-V single cycle floating point
  layouts = {{"Standard",
              ":/layouts/RISC-V/rvss_float/rv_ss_float_standard_layout.json",
              {{{0, 0}, QPointF{0.5, 0}}}},
             {"Extended",
              ":/layouts/RISC-V/rvss_float/rv_ss_float_extended_layout.json",
              {{{0, 0}, QPointF{0.5, 0}}}}};
  add32And64BitVariations<vsrtl::core::RVSS_FLOAT>(
      rv_ss_info, "Single-cycle floating point processor",
      "A single cycle floating point processor", Variations::RV_SS::RV32F,
      Variations::RV_SS::RV64F, {}, layouts, defaultRegVals);

  return rv_ss_info;
}

static ProcClassInfo register_rv_mc() {
  std::vector<Layout> layouts;

  ProcClassInfo rv_mc_info(ProcessorID::RV_5MC, ISA::RV32I,
                           QStringLiteral("Multi-cycle RISC-V"),
                           Variations::RV_5MC::RV32I_1M);

  // RISC-V multicycle, one memory
  layouts = {{"Extended",
              ":/layouts/RISC-V/rv5mc/rv5mc_1m_extended_layout.json", // TODO
              {{{0, 0}, QPointF{0.08, 0}},
               {{0, 1}, QPointF{0.28, 0}},
               {{0, 2}, QPointF{0.54, 0}},
               {{0, 3}, QPointF{0.78, 0}},
               {{0, 4}, QPointF{0.9, 0}}}}};

  add32And64BitVariations<vsrtl::core::RV5MC1M>(
      rv_mc_info, "Multi-cycle processor with one memory", rv5mc_desc_1m,
      Variations::RV_5MC::RV32I_1M, Variations::RV_5MC::RV64I_1M, {}, layouts,
      defaultRegVals);

  // RISC-V multicycle, separate memories
  layouts = {{"Extended",
              ":/layouts/RISC-V/rv5mc/rv5mc_extended_layout.json",
              {{{0, 0}, QPointF{0.08, 0}},
               {{0, 1}, QPointF{0.28, 0}},
               {{0, 2}, QPointF{0.54, 0}},
               {{0, 3}, QPointF{0.78, 0}},
               {{0, 4}, QPointF{0.9, 0}}}}};

  add32And64BitVariations<vsrtl::core::RV5MC2M>(
      rv_mc_info, "Multi-cycle processor with separate memories", rv5mc_desc,
      Variations::RV_5MC::RV32I_2M, Variations::RV_5MC::RV64I_2M,
      {"Two Memories"}, layouts, defaultRegVals);

  return rv_mc_info;
}

static ProcClassInfo register_rv_5s() {
  std::vector<Layout> layouts;

  const QString option_Hazard = "Hazard Detection Unit";
  const QString option_Forwarding = "Forwarding Unit";

  ProcClassInfo rv5s_info(ProcessorID::RV_5S, ISA::RV32I,
                          QStringLiteral("5-Stage RISC-V"),
                          Variations::RV_5S::RV32I_FU_HU);

  //------------------------------------------------------------------------------
  // RISC-V 5-stage without forwarding or hazard detection
  //------------------------------------------------------------------------------
  layouts = {
      {"Standard",
       ":/layouts/RISC-V/rv5s_no_fw_hz/rv5s_no_fw_hz_standard_layout.json",
       {{{0, 0}, QPointF{0.08, 0}},
        {{0, 1}, QPointF{0.3, 0}},
        {{0, 2}, QPointF{0.54, 0}},
        {{0, 3}, QPointF{0.73, 0}},
        {{0, 4}, QPointF{0.88, 0}}}},
      {"Extended",
       ":/layouts/RISC-V/rv5s_no_fw_hz/rv5s_no_fw_hz_extended_layout.json",
       {{{0, 0}, QPointF{0.08, 0.0}},
        {{0, 1}, QPointF{0.31, 0.0}},
        {{0, 2}, QPointF{0.56, 0.0}},
        {{0, 3}, QPointF{0.76, 0.0}},
        {{0, 4}, QPointF{0.9, 0.0}}}}};
  add32And64BitVariations<vsrtl::core::RV5S_NO_FW_HZ>(
      rv5s_info, "5-stage processor w/o forwarding or hazard detection",
      rv5s_no_fw_hz_desc, Variations::RV_5S::RV32I, Variations::RV_5S::RV64I,
      {}, layouts, defaultRegVals);

  //------------------------------------------------------------------------------
  // RISC-V 5-stage with floating point unit without forwarding or hazard
  // detection
  //------------------------------------------------------------------------------
  layouts = {{"Standard",
              ":/layouts/RISC-V/rv5s_float_no_fw_hz/"
              "rv5s_float_no_fw_hz_standard_layout.json",
              {{{0, 0}, QPointF{0.08, 0}},
               {{0, 1}, QPointF{0.35, 0}},
               {{0, 2}, QPointF{0.60, 0}},
               {{0, 3}, QPointF{0.76, 0}},
               {{0, 4}, QPointF{0.90, 0}}}},
             {"Extended",
              ":/layouts/RISC-V/rv5s_float_no_fw_hz/"
              "rv5s_float_no_fw_hz_extended_layout.json",
              {{{0, 0}, QPointF{0.08, 0.0}},
               {{0, 1}, QPointF{0.32, 0.0}},
               {{0, 2}, QPointF{0.59, 0.0}},
               {{0, 3}, QPointF{0.78, 0.0}},
               {{0, 4}, QPointF{0.91, 0.0}}}}};
  add32And64BitVariations<vsrtl::core::RV5S_FLOAT_NO_FW_HZ>(
      rv5s_info,
      "5-stage floating point processor w/o forwarding or hazard detection",
      rv5s_float_no_fw_hz_desc, Variations::RV_5S::RV32F,
      Variations::RV_5S::RV64F, {}, layouts, defaultRegVals);

  //------------------------------------------------------------------------------
  // RISC-V 5-stage without hazard detection (with forwarding)
  //------------------------------------------------------------------------------
  layouts = {{"Standard",
              ":/layouts/RISC-V/rv5s_no_hz/rv5s_no_hz_standard_layout.json",
              {{{0, 0}, QPointF{0.08, 0}},
               {{0, 1}, QPointF{0.3, 0}},
               {{0, 2}, QPointF{0.53, 0}},
               {{0, 3}, QPointF{0.75, 0}},
               {{0, 4}, QPointF{0.88, 0}}}},
             {"Extended",
              ":/layouts/RISC-V/rv5s_no_hz/rv5s_no_hz_extended_layout.json",
              {{{0, 0}, QPointF{0.08, 0}},
               {{0, 1}, QPointF{0.28, 0}},
               {{0, 2}, QPointF{0.53, 0}},
               {{0, 3}, QPointF{0.78, 0}},
               {{0, 4}, QPointF{0.9, 0}}}}};
  add32And64BitVariations<vsrtl::core::RV5S_NO_HZ>(
      rv5s_info, "5-stage processor w/o hazard detection", rv5s_no_hz_desc,
      Variations::RV_5S::RV32I_FU, Variations::RV_5S::RV64I_FU,
      {option_Forwarding}, layouts, defaultRegVals);

  //------------------------------------------------------------------------------
  // RISC-V 5-stage without forwarding unit (with hazard detection)
  //------------------------------------------------------------------------------
  layouts = {{"Standard",
              ":/layouts/RISC-V/rv5s_no_fw/rv5s_no_fw_standard_layout.json",
              {{{0, 0}, QPointF{0.08, 0}},
               {{0, 1}, QPointF{0.3, 0}},
               {{0, 2}, QPointF{0.53, 0}},
               {{0, 3}, QPointF{0.75, 0}},
               {{0, 4}, QPointF{0.88, 0}}}},
             {"Extended",
              ":/layouts/RISC-V/rv5s_no_fw/rv5s_no_fw_extended_layout.json",
              {{{0, 0}, QPointF{0.08, 0}},
               {{0, 1}, QPointF{0.28, 0}},
               {{0, 2}, QPointF{0.53, 0}},
               {{0, 3}, QPointF{0.78, 0}},
               {{0, 4}, QPointF{0.9, 0}}}}};
  add32And64BitVariations<vsrtl::core::RV5S_NO_FW>(
      rv5s_info, "5-stage processor w/o forwarding unit", rv5s_no_fw_desc,
      Variations::RV_5S::RV32I_HU, Variations::RV_5S::RV64I_HU, {option_Hazard},
      layouts, defaultRegVals);

  //------------------------------------------------------------------------------
  // RISC-V 5-stage
  //------------------------------------------------------------------------------
  layouts = {{"Standard",
              ":/layouts/RISC-V/rv5s/rv5s_standard_layout.json",
              {{{0, 0}, QPointF{0.08, 0}},
               {{0, 1}, QPointF{0.29, 0}},
               {{0, 2}, QPointF{0.55, 0}},
               {{0, 3}, QPointF{0.75, 0}},
               {{0, 4}, QPointF{0.87, 0}}}},
             {"Extended",
              ":/layouts/RISC-V/rv5s/rv5s_extended_layout.json",
              {{{0, 0}, QPointF{0.08, 0}},
               {{0, 1}, QPointF{0.28, 0}},
               {{0, 2}, QPointF{0.54, 0}},
               {{0, 3}, QPointF{0.78, 0}},
               {{0, 4}, QPointF{0.9, 0}}}}};
  add32And64BitVariations<vsrtl::core::RV5S>(
      rv5s_info, "5-stage processor", rv5s_desc, Variations::RV_5S::RV32I_FU_HU,
      Variations::RV_5S::RV64I_FU_HU, {option_Hazard, option_Forwarding},
      layouts, defaultRegVals);

  return rv5s_info;
}

static ProcClassInfo register_rv_6s() {
  std::vector<Layout> layouts;

  ProcClassInfo rv6s_info(ProcessorID::RV_6S_DUAL, ISA::RV32I,
                          QStringLiteral("6-Stage Dual-Issue RISC-V"),
                          Variations::RV_6S_DUAL::RV32I);

  layouts = {{"Extended",
              ":/layouts/RISC-V/rv6s_dual/rv6s_dual_extended_layout.json",
              {{{{0, 0}, QPointF{0.06, 0}},
                {{1, 0}, QPointF{0.06, 1}},
                {{0, 1}, QPointF{0.22, 0}},
                {{1, 1}, QPointF{0.22, 1}},
                {{0, 2}, QPointF{0.40, 0}},
                {{1, 2}, QPointF{0.40, 1}},
                {{0, 3}, QPointF{0.59, 0}},
                {{1, 3}, QPointF{0.59, 1}},
                {{0, 4}, QPointF{0.80, 0}},
                {{1, 4}, QPointF{0.80, 1}},
                {{0, 5}, QPointF{0.90, 0}},
                {{1, 5}, QPointF{0.90, 1}}}}}};
  add32And64BitVariations<vsrtl::core::RV6S_DUAL>(
      rv6s_info, "6-stage dual-issue processor", rv6s_desc,
      Variations::RV_6S_DUAL::RV32I, Variations::RV_6S_DUAL::RV64I, {}, layouts,
      defaultRegVals);

  return rv6s_info;
}

ProcessorRegistry::ProcessorRegistry() {
  addProcessor(register_rv_ss());
  addProcessor(register_rv_mc());
  addProcessor(register_rv_5s());
  addProcessor(register_rv_6s());
}
} // namespace Ripes
