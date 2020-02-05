#pragma once

#include <QMetaType>
#include <map>
#include <memory>

#include "isainfo.h"
#include "processors/ripesprocessor.h"

#include "processors/RISC-V/rv5s/rv5s.h"
#include "processors/RISC-V/rv5swof/rv5swof.h"
#include "processors/RISC-V/rvss/rvss.h"

namespace Ripes {

// =============================== Processors =================================
enum class ProcessorID { RISCV_SS, RISCV_5S, RISCV_5S_WOF };
// ============================================================================

using RegisterInitialization = std::map<unsigned, uint32_t>;
struct Layout {
    QString name;
    QString file;
    /**
     * @brief stageLabelPositions
     * Stage labels are not a part of the VSRTL processor model, and as such are not serialized within the models
     * layout. stageLabelPositions determines the position of stage labels as a relative distance based on the processor
     * models' width in the VSRTL view. Should be in the range [0;1].
     * Must contain an entry for each stage in the processor model.
     */
    std::vector<double> stageLabelPositions;
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
    static const ProcessorDescription& getDescription(ProcessorID id) { return instance().m_descriptions[id]; }
    static std::unique_ptr<vsrtl::core::RipesProcessor> constructProcessor(ProcessorID id) {
        switch (id) {
            case ProcessorID::RISCV_5S_WOF:
                return std::make_unique<vsrtl::core::RV5SWOF>();
            case ProcessorID::RISCV_5S:
                return std::make_unique<vsrtl::core::RV5S>();
            case ProcessorID::RISCV_SS: {
                return std::make_unique<vsrtl::core::RVSS>();
            }
        }
    }

private:
    ProcessorRegistry() {
        // Initialize processors

        // RISC-V single cycle
        ProcessorDescription desc;
        desc.id = ProcessorID::RISCV_SS;
        desc.isa = ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V Single Cycle Processor";
        desc.description = "A single cycle RISC-V processor";
        desc.layouts = {{"Standard", ":/layouts/RISC-V/rvss/rv_ss_standard_layout.json", {0.5}},
                        {"Extended", ":/layouts/RISC-V/rvss/rv_ss_extended_layout.json", {0.5}}};
        desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        m_descriptions[desc.id] = desc;

        // RISC-V 5-Stage with forwarding
        desc = ProcessorDescription();
        desc.id = ProcessorID::RISCV_5S;
        desc.isa = ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V 5-Stage Processor";
        desc.description = "A 5-Stage in-order RISC-V processor with hazard detection and forwarding.";
        desc.layouts = {{"Standard", ":/layouts/RISC-V/rv5s/rv5s_standard_layout.json", {0.11, 0.31, 0.55, 0.77, 0.92}},
                        {"Extended", ":/layouts/RISC-V/rv5s/rv5s_extended_layout.json", {0.11, 0.31, 0.58, 0.8, 0.94}}};
        desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        m_descriptions[desc.id] = desc;

        // RISC-V 5-stage without forwarding
        desc = ProcessorDescription();
        desc.id = ProcessorID::RISCV_5S_WOF;
        desc.isa = ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V 5-Stage Processor wo/ forwarding";
        desc.description =
            "A 5-Stage in-order RISC-V processor with no hazard detection and forwarding. \n\nThe user is expected to "
            "resolve all data hazard through insertions of NOP instructions to correctly schedule the code.";
        desc.layouts = {
            {"Standard", ":/layouts/RISC-V/rv5swof/rv5swof_standard_layout.json", {0.12, 0.35, 0.58, 0.75, 0.92}},
            {"Extended", ":/layouts/RISC-V/rv5swof/rv5swof_extended_layout.json", {0.11, 0.32, 0.55, 0.80, 0.94}}};
        desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        m_descriptions[desc.id] = desc;
    }

    static ProcessorRegistry& instance() {
        static ProcessorRegistry pr;
        return pr;
    }

    ProcessorMap m_descriptions;
};
}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::ProcessorID);
