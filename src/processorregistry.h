#pragma once

#include <QMetaType>
#include <QPolygonF>
#include <map>
#include <memory>

#include "isa/isainfo.h"
#include "processors/interface/ripesprocessor.h"

#include "processors/RISC-V/rv5s/rv5s.h"
#include "processors/RISC-V/rv5s_no_fw/rv5s_no_fw.h"
#include "processors/RISC-V/rv5s_no_fw_hz/rv5s_no_fw_hz.h"
#include "processors/RISC-V/rv5s_no_hz/rv5s_no_hz.h"
#include "processors/RISC-V/rv6s_dual/rv6s_dual.h"
#include "processors/RISC-V/rvss/rvss.h"

namespace Ripes {
Q_NAMESPACE

// =============================== Processors =================================
// The order of the ProcessorID enum defines the order of which the processors will appear in the processor selection
// dialog.
enum ProcessorID {
    RV32_SS,
    RV32_5S_NO_FW_HZ,
    RV32_5S_NO_HZ,
    RV32_5S_NO_FW,
    RV32_5S,
    RV32_6S_DUAL,
    RV64_SS,
    RV64_5S_NO_FW_HZ,
    RV64_5S_NO_HZ,
    RV64_5S_NO_FW,
    RV64_5S,
    RV64_6S_DUAL,
    NUM_PROCESSORS
};
Q_ENUM_NS(Ripes::ProcessorID);  // Register with the metaobject system
// ============================================================================

using RegisterInitialization = std::map<unsigned, uint32_t>;
struct Layout {
    QString name;
    QString file;
    /**
     * @brief stageLabelPositions
     * Stage labels are not a part of the VSRTL processor model, and as such are not serialized within the models
     * layout. The first value in the points determines the position of stage labels as a relative distance based on the
     * processor models' width in the VSRTL view. Should be in the range [0;1]. The second value in the point determines
     * the y-position of the label, as a multiple of the height of the font used. This is used so that multiple labels
     * can be "stacked" over one another. Must contain an entry for each stage in the processor model.
     */
    std::vector<QPointF> stageLabelPositions;
    bool operator==(const Layout& rhs) const { return this->name == rhs.name; }
};

struct ProcessorDescription {
    ProcessorID id;
    RegisterInitialization defaultRegisterVals;
    const ISAInfoBase* isa;
    QString name;
    QString description;
    std::vector<Layout> layouts;
};

class ProcessorRegistry {
public:
    using ProcessorMap = std::map<ProcessorID, ProcessorDescription>;
    static const ProcessorMap& getAvailableProcessors() { return instance().m_descriptions; }
    static const ProcessorDescription& getDescription(ProcessorID id) {
        auto desc = instance().m_descriptions.find(id);
        if (desc == instance().m_descriptions.end()) {
            return instance().m_descriptions.begin()->second;
        }
        return desc->second;
    }
    static std::unique_ptr<RipesProcessor> constructProcessor(ProcessorID id, const QStringList& extensions) {
        switch (id) {
            case ProcessorID::RV32_5S_NO_FW_HZ:
                return std::make_unique<vsrtl::core::RV5S_NO_FW_HZ<32>>(extensions);
            case ProcessorID::RV32_5S:
                return std::make_unique<vsrtl::core::RV5S<32>>(extensions);
            case ProcessorID::RV32_6S_DUAL:
                return std::make_unique<vsrtl::core::RV6S_DUAL<32>>(extensions);
            case ProcessorID::RV32_SS:
                return std::make_unique<vsrtl::core::RVSS<32>>(extensions);
            case ProcessorID::RV32_5S_NO_HZ:
                return std::make_unique<vsrtl::core::RV5S_NO_HZ<32>>(extensions);
            case ProcessorID::RV32_5S_NO_FW:
                return std::make_unique<vsrtl::core::RV5S_NO_FW<32>>(extensions);
            case ProcessorID::RV64_5S_NO_FW_HZ:
                return std::make_unique<vsrtl::core::RV5S_NO_FW_HZ<64>>(extensions);
            case ProcessorID::RV64_5S:
                return std::make_unique<vsrtl::core::RV5S<64>>(extensions);
            case ProcessorID::RV64_6S_DUAL:
                return std::make_unique<vsrtl::core::RV6S_DUAL<64>>(extensions);
            case ProcessorID::RV64_SS:
                return std::make_unique<vsrtl::core::RVSS<64>>(extensions);
            case ProcessorID::RV64_5S_NO_HZ:
                return std::make_unique<vsrtl::core::RV5S_NO_HZ<64>>(extensions);
            case ProcessorID::RV64_5S_NO_FW:
                return std::make_unique<vsrtl::core::RV5S_NO_FW<64>>(extensions);
            case ProcessorID::NUM_PROCESSORS:
            default:
                Q_UNREACHABLE();
        }
    }

private:
    ProcessorRegistry() {
        // Initialize processors

        for (int xlen : {32, 64}) {
            // RISC-V single cycle
            ProcessorDescription desc;
            desc.id = xlen == 32 ? ProcessorID::RV32_SS : ProcessorID::RV64_SS;
            desc.isa = xlen == 32 ? vsrtl::core::RVSS<32>::ISA() : vsrtl::core::RVSS<64>::ISA();
            desc.name = "Single-cycle processor";
            desc.description = "A single cycle processor";
            desc.layouts = {{"Standard", ":/layouts/RISC-V/rvss/rv_ss_standard_layout.json", {QPointF{0.5, 0}}},
                            {"Extended", ":/layouts/RISC-V/rvss/rv_ss_extended_layout.json", {QPointF{0.5, 0}}}};
            desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
            m_descriptions[desc.id] = desc;

            // RISC-V 5-stage without forwarding or hazard detection
            desc = ProcessorDescription();
            desc.id = xlen == 32 ? ProcessorID::RV32_5S_NO_FW_HZ : ProcessorID::RV64_5S_NO_FW_HZ;
            desc.isa = xlen == 32 ? vsrtl::core::RV5S_NO_FW_HZ<32>::ISA() : vsrtl::core::RV5S_NO_FW_HZ<64>::ISA();
            desc.name = "5-stage processor w/o forwarding or hazard detection";
            desc.description = "A 5-stage in-order processor with no forwarding or hazard detection/elimination.";
            desc.layouts = {
                {"Standard",
                 ":/layouts/RISC-V/rv5s_no_fw_hz/rv5s_no_fw_hz_standard_layout.json",
                 {QPointF{0.08, 0}, QPointF{0.3, 0}, QPointF{0.54, 0}, QPointF{0.73, 0}, QPointF{0.88, 0}}},
                {"Extended",
                 ":/layouts/RISC-V/rv5s_no_fw_hz/rv5s_no_fw_hz_extended_layout.json",
                 {QPointF{0.08, 0.0}, QPointF{0.31, 0.0}, QPointF{0.56, 0.0}, QPointF{0.76, 0.0}, QPointF{0.9, 0.0}}}};

            desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
            m_descriptions[desc.id] = desc;

            // RISC-V 5-stage without hazard detection
            desc = ProcessorDescription();
            desc.id = xlen == 32 ? ProcessorID::RV32_5S_NO_HZ : ProcessorID::RV64_5S_NO_HZ;
            desc.isa = xlen == 32 ? vsrtl::core::RV5S_NO_HZ<32>::ISA() : vsrtl::core::RV5S_NO_HZ<64>::ISA();
            desc.name = "5-stage processor w/o hazard detection";
            desc.description = "A 5-stage in-order processor with forwarding but no hazard detection/elimination.";
            desc.layouts = {
                {"Standard",
                 ":/layouts/RISC-V/rv5s_no_hz/rv5s_no_hz_standard_layout.json",
                 {QPointF{0.08, 0}, QPointF{0.3, 0}, QPointF{0.53, 0}, QPointF{0.75, 0}, QPointF{0.88, 0}}},
                {"Extended",
                 ":/layouts/RISC-V/rv5s_no_hz/rv5s_no_hz_extended_layout.json",
                 {QPointF{0.08, 0}, QPointF{0.28, 0}, QPointF{0.53, 0}, QPointF{0.78, 0}, QPointF{0.9, 0}}}};
            desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
            m_descriptions[desc.id] = desc;

            // RISC-V 5-stage without forwarding unit
            desc = ProcessorDescription();
            desc.id = xlen == 32 ? ProcessorID::RV32_5S_NO_FW : ProcessorID::RV64_5S_NO_FW;
            desc.isa = xlen == 32 ? vsrtl::core::RV5S_NO_FW<32>::ISA() : vsrtl::core::RV5S_NO_FW<64>::ISA();
            desc.name = "5-Stage processor w/o forwarding unit";
            desc.description = "A 5-stage in-order processor with hazard detection/elimination but no forwarding unit.";
            desc.layouts = {
                {"Standard",
                 ":/layouts/RISC-V/rv5s_no_fw/rv5s_no_fw_standard_layout.json",
                 {QPointF{0.08, 0}, QPointF{0.3, 0}, QPointF{0.53, 0}, QPointF{0.75, 0}, QPointF{0.88, 0}}},
                {"Extended",
                 ":/layouts/RISC-V/rv5s_no_fw/rv5s_no_fw_extended_layout.json",
                 {QPointF{0.08, 0}, QPointF{0.28, 0}, QPointF{0.53, 0}, QPointF{0.78, 0}, QPointF{0.9, 0}}}};
            desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
            m_descriptions[desc.id] = desc;

            // RISC-V 5-stage
            desc = ProcessorDescription();
            desc.id = xlen == 32 ? ProcessorID::RV32_5S : ProcessorID::RV64_5S;
            desc.isa = xlen == 32 ? vsrtl::core::RV5S<32>::ISA() : vsrtl::core::RV5S<64>::ISA();
            desc.name = "5-stage processor";
            desc.description = "A 5-stage in-order processor with hazard detection/elimination and forwarding.";
            desc.layouts = {
                {"Standard",
                 ":/layouts/RISC-V/rv5s/rv5s_standard_layout.json",
                 {QPointF{0.08, 0}, QPointF{0.29, 0}, QPointF{0.55, 0}, QPointF{0.75, 0}, QPointF{0.87, 0}}},
                {"Extended",
                 ":/layouts/RISC-V/rv5s/rv5s_extended_layout.json",
                 {QPointF{0.08, 0}, QPointF{0.28, 0}, QPointF{0.54, 0}, QPointF{0.78, 0}, QPointF{0.9, 0}}}};
            desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
            m_descriptions[desc.id] = desc;

            // RISC-V 6-stage dual issue
            desc = ProcessorDescription();
            desc.id = xlen == 32 ? ProcessorID::RV32_6S_DUAL : ProcessorID::RV64_6S_DUAL;
            desc.isa = xlen == 32 ? vsrtl::core::RV6S_DUAL<32>::ISA() : vsrtl::core::RV6S_DUAL<64>::ISA();
            desc.name = "6-stage dual-issue processor";
            desc.description =
                "A 6-stage dual-issue in-order processor. Each way may execute arithmetic instructions, whereas way 1 "
                "is "
                "reserved for controlflow and ecall instructions, and way 2 for memory accessing instructions.";
            desc.layouts = {{"Extended",
                             ":/layouts/RISC-V/rv6s_dual/rv6s_dual_extended_layout.json",
                             {{QPointF{0.06, 0}, QPointF{0.06, 1}, QPointF{0.22, 0}, QPointF{0.22, 1}, QPointF{0.35, 0},
                               QPointF{0.35, 1}, QPointF{0.54, 0}, QPointF{0.54, 1}, QPointF{0.78, 0}, QPointF{0.78, 1},
                               QPointF{0.87, 0}, QPointF{0.87, 1}}}}};
            desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
            m_descriptions[desc.id] = desc;
        }
    }

    static ProcessorRegistry& instance() {
        static ProcessorRegistry pr;
        return pr;
    }

    ProcessorMap m_descriptions;
};  // namespace Ripes
}  // namespace Ripes
