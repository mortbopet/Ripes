#pragma once

#include <QMetaType>
#include <map>
#include <memory>

#include "isainfo.h"
#include "processors/ripesprocessor.h"

#include "processors/ss/riscv_ss.h"

namespace Ripes {

// =============================== Processors =================================
enum class ProcessorID { RISCV_SS, RISCV_5S_WF, RISCV_5S_WOF };
// ============================================================================

enum class ProgramSegment { Text, Data, Stack };
const static std::map<ProgramSegment, QString> s_programSegmentName = {{ProgramSegment::Text, "Text"},
                                                                       {ProgramSegment::Data, "Data"},
                                                                       {ProgramSegment::Stack, "Stack"}};
struct ProcessorSetup {
    ProcessorID id;
    std::map<ProgramSegment, uint32_t> segmentPtrs;
};

struct ProcessorDescription {
    ProcessorSetup defaultSetup;
    const ISAInfoBase* isa;
    QString name;
    QString description;
};

class ProcessorRegistry {
public:
    using ProcessorMap = std::map<ProcessorID, ProcessorDescription>;
    static const ProcessorMap& getAvailableProcessors() { return instance().m_descriptions; }
    static const ProcessorDescription& getDescription(ProcessorID id) { return instance().m_descriptions[id]; }
    static std::unique_ptr<vsrtl::core::RipesProcessor> constructProcessor(ProcessorID id) {
        switch (id) {
            case ProcessorID::RISCV_5S_WOF:
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
        desc.defaultSetup.id = ProcessorID::RISCV_SS;
        desc.defaultSetup.segmentPtrs[ProgramSegment::Text] = 0;
        desc.defaultSetup.segmentPtrs[ProgramSegment::Data] = 0x10000000;
        desc.defaultSetup.segmentPtrs[ProgramSegment::Stack] = 0x7ffffff0;

        desc.isa = ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V Single Cycle";
        desc.description = "A single cycle RISC-V processor";
        m_descriptions[desc.defaultSetup.id] = desc;

        // RISC-V 5-Stage with forwarding
        desc.defaultSetup.id = ProcessorID::RISCV_5S_WF;
        desc.defaultSetup.segmentPtrs[ProgramSegment::Text] = 0;
        desc.defaultSetup.segmentPtrs[ProgramSegment::Data] = 0x10000000;
        desc.defaultSetup.segmentPtrs[ProgramSegment::Stack] = 0x7ffffff0;
        desc.isa = ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V 5-Stage w/ forwarding";
        desc.description = "A 5-Stage in-order RISC-V processor with hazard detection and forwarding.";
        m_descriptions[desc.defaultSetup.id] = desc;

        // RISC-V 5-stage without forwarding
        desc.defaultSetup.id = ProcessorID::RISCV_5S_WOF;
        desc.defaultSetup.segmentPtrs[ProgramSegment::Text] = 0;
        desc.defaultSetup.segmentPtrs[ProgramSegment::Data] = 0x10000000;
        desc.defaultSetup.segmentPtrs[ProgramSegment::Stack] = 0x7ffffff0;
        desc.isa = ISAInfo<ISA::RV32IM>::instance();
        desc.name = "RISC-V 5-Stage wo/ forwarding";
        desc.description =
            "A 5-Stage in-order RISC-V processor with no hazard detection and forwarding. \n\nThe user is expected to "
            "resolve all data hazard through insertions of NOP instructions to correctly schedule the code.";
        m_descriptions[desc.defaultSetup.id] = desc;
    }

    static ProcessorRegistry& instance() {
        static ProcessorRegistry pr;
        return pr;
    }

    ProcessorMap m_descriptions;
};
}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::ProcessorID);
Q_DECLARE_METATYPE(Ripes::ProgramSegment);
