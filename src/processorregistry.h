#pragma once

#include <QMetaType>
#include <map>
#include <memory>

#include "isainfo.h"

#include "processors/ripesprocessor.h"
// Processors
#include "processors/ss/riscv_ss.h"

enum class ProcessorID { RISCV_SS, RISCV_5S_WF, RISCV_5S_WOF };

struct ProcessorDescription {
    ProcessorID id;
    ISAInfoBase* isa;
    QString name;
    QString description;
};

Q_DECLARE_METATYPE(ProcessorID);

class ProcessorRegistry {
public:
    using ProcessorMap = std::map<ProcessorID, ProcessorDescription>;
    static const ProcessorMap& getAvailableProcessors() { return instance().m_descriptions; }
    static std::unique_ptr<Ripes::RipesProcessor> constructProcessor(ProcessorID id) {
        switch (id) {
            case ProcessorID::RISCV_5S_WOF:
            case ProcessorID::RISCV_5S_WF:
            case ProcessorID::RISCV_SS: {
                return std::make_unique<vsrtl::RISCV::SingleCycleRISCV>();
            }
        }
    }

private:
    ProcessorRegistry() {
        // Initialize processors

        // RISC-V single cycle
        ProcessorDescription desc;
        desc.id = ProcessorID::RISCV_SS;
        desc.isa = &ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V Single Cycle";
        desc.description = "A single cycle RISC-V processor";
        m_descriptions[desc.id] = desc;

        // RISC-V 5-Stage with forwarding
        desc.id = ProcessorID::RISCV_5S_WF;
        desc.isa = &ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V 5-Stage w/ forwarding";
        desc.description = "A 5-Stage in-order RISC-V processor with hazard detection and forwarding.";
        m_descriptions[desc.id] = desc;

        // RISC-V 5-stage without forwarding
        desc.id = ProcessorID::RISCV_5S_WOF;
        desc.isa = &ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V 5-Stage wo/ forwarding";
        desc.description =
            "A 5-Stage in-order RISC-V processor with no hazard detection and forwarding. \n\nThe user is expected to "
            "resolve all data hazard through insertions of NOP instructions to correctly schedule the code.";
        m_descriptions[desc.id] = desc;
    }

    static ProcessorRegistry& instance() {
        static ProcessorRegistry pr;
        return pr;
    }

    ProcessorMap m_descriptions;
};
