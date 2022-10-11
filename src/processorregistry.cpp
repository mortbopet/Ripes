#include "processorregistry.h"

#include <QPolygonF>

#include "processors/RISC-V/rv5s/rv5s.h"
#include "processors/RISC-V/rv5s_no_fw/rv5s_no_fw.h"
#include "processors/RISC-V/rv5s_no_fw_hz/rv5s_no_fw_hz.h"
#include "processors/RISC-V/rv5s_no_hz/rv5s_no_hz.h"
#include "processors/RISC-V/rv6s_dual/rv6s_dual.h"
#include "processors/RISC-V/rvss/rvss.h"

namespace Ripes {

// TODO: This whole registration logic is ugly and should be reworked - either
// load a JSON file at runtime, or have a small codegen pass during compilation
// to generate this file based on an input JSON.

#define no_hz_note                                                             \
  "<br><b>NOTE: given the lack of a hazard unit (and "                         \
  "therefore the lack of dynamic hazard resolution in the pipeline) programs " \
  "with hazards will execute incorrectly, unless resolved by inserting "       \
  "nop's to manually stall the pipeline.</b>";

constexpr const char rv5s_no_fw_hz_desc[] =
    "A 5-stage in-order processor with no forwarding or hazard "
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

ProcessorRegistry::ProcessorRegistry() {
  // Initialize processors
  std::vector<Layout> layouts;
  RegisterInitialization defRegVals;

  // RISC-V single cycle
  layouts = {{"Standard",
              ":/layouts/RISC-V/rvss/rv_ss_standard_layout.json",
              {{{0, 0}, QPointF{0.5, 0}}}},
             {"Extended",
              ":/layouts/RISC-V/rvss/rv_ss_extended_layout.json",
              {{{0, 0}, QPointF{0.5, 0}}}}};
  defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
  addProcessor(ProcInfo<vsrtl::core::RVSS<uint32_t>>(
      ProcessorID::RV32_SS, "Single-cycle processor",
      "A single cycle processor", layouts, defRegVals));
  addProcessor(ProcInfo<vsrtl::core::RVSS<uint64_t>>(
      ProcessorID::RV64_SS, "Single-cycle processor",
      "A single cycle processor", layouts, defRegVals));

  // RISC-V 5-stage without forwarding or hazard detection
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
  defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
  addProcessor(ProcInfo<vsrtl::core::RV5S_NO_FW_HZ<uint32_t>>(
      ProcessorID::RV32_5S_NO_FW_HZ,
      "5-stage processor w/o forwarding or hazard detection",
      rv5s_no_fw_hz_desc, layouts, defRegVals));
  addProcessor(ProcInfo<vsrtl::core::RV5S_NO_FW_HZ<uint64_t>>(
      ProcessorID::RV64_5S_NO_FW_HZ,
      "5-stage processor w/o forwarding or hazard detection",
      rv5s_no_fw_hz_desc, layouts, defRegVals));

  // RISC-V 5-stage without hazard detection
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
  defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
  addProcessor(ProcInfo<vsrtl::core::RV5S_NO_HZ<uint32_t>>(
      ProcessorID::RV32_5S_NO_HZ, "5-stage processor w/o hazard detection",
      rv5s_no_hz_desc, layouts, defRegVals));
  addProcessor(ProcInfo<vsrtl::core::RV5S_NO_HZ<uint64_t>>(
      ProcessorID::RV64_5S_NO_HZ, "5-stage processor w/o hazard detection",
      rv5s_no_hz_desc, layouts, defRegVals));

  // RISC-V 5-stage without forwarding unit
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
  defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
  addProcessor(ProcInfo<vsrtl::core::RV5S_NO_FW<uint32_t>>(
      ProcessorID::RV32_5S_NO_FW, "5-Stage processor w/o forwarding unit",
      rv5s_no_fw_desc, layouts, defRegVals));
  addProcessor(ProcInfo<vsrtl::core::RV5S_NO_FW<uint64_t>>(
      ProcessorID::RV64_5S_NO_FW, "5-Stage processor w/o forwarding unit",
      rv5s_no_fw_desc, layouts, defRegVals));

  // RISC-V 5-stage
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
  defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
  addProcessor(ProcInfo<vsrtl::core::RV5S<uint32_t>>(
      ProcessorID::RV32_5S, "5-stage processor", rv5s_desc, layouts,
      defRegVals));
  addProcessor(ProcInfo<vsrtl::core::RV5S<uint64_t>>(
      ProcessorID::RV64_5S, "5-stage processor", rv5s_desc, layouts,
      defRegVals));

  // RISC-V 6-stage dual issue
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
  defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
  addProcessor(ProcInfo<vsrtl::core::RV6S_DUAL<uint32_t>>(
      ProcessorID::RV32_6S_DUAL, "6-stage dual-issue processor", rv6s_desc,
      layouts, defRegVals));
  addProcessor(ProcInfo<vsrtl::core::RV6S_DUAL<uint64_t>>(
      ProcessorID::RV64_6S_DUAL, "6-stage dual-issue processor", rv6s_desc,
      layouts, defRegVals));
}
} // namespace Ripes
