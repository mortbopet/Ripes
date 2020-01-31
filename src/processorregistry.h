#pragma once

#include <QMetaType>
#include <map>
#include <memory>

#include "isainfo.h"
#include "processors/ripesprocessor.h"

#include "processors/5s/riscv_5s.h"
#include "processors/ss/riscv_ss.h"

namespace Ripes {

// =============================== Processors =================================
enum class ProcessorID { RISCV_SS, RISCV_5S_WF, RISCV_5S_WOF };
// ============================================================================

using RegisterInitialization = std::map<unsigned, uint32_t>;
struct Layout {
    QString name;
    QString file;
    bool showPortWidth;
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
            case ProcessorID::RISCV_5S_WOF: {
                return std::make_unique<vsrtl::core::FiveStageRISCV>();
            }
            case ProcessorID::RISCV_5S_WF:
            case ProcessorID::RISCV_SS: {
                return std::make_unique<vsrtl::core::SingleCycleRISCV>();
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
        desc.name = "RISC-V Single Cycle";
        desc.description = "A single cycle RISC-V processor";
        desc.layouts = {{"Standard", ":/layouts/ss/rv_ss_standard_layout.json", false},
                        {"Extended", ":/layouts/ss/rv_ss_extended_layout.json", true}};
        desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        m_descriptions[desc.id] = desc;

        // RISC-V 5-Stage with forwarding
        desc = ProcessorDescription();
        desc.id = ProcessorID::RISCV_5S_WF;
        desc.isa = ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V 5-Stage w/ forwarding";
        desc.description = "A 5-Stage in-order RISC-V processor with hazard detection and forwarding.";
        desc.layouts.push_back({"Standard", ":/layouts/ss/rv_ss_layout.json", false});
        desc.defaultRegisterVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        m_descriptions[desc.id] = desc;

        // RISC-V 5-stage without forwarding
        desc = ProcessorDescription();
        desc.id = ProcessorID::RISCV_5S_WOF;
        desc.isa = ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V 5-Stage wo/ forwarding";
        desc.description =
            "A 5-Stage in-order RISC-V processor with no hazard detection and forwarding. \n\nThe user is expected to "
            "resolve all data hazard through insertions of NOP instructions to correctly schedule the code.";
        desc.layouts.push_back({});  //"Extended", ":/layouts/5s/rv_5s_wof_extended_layout.json", true});
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
