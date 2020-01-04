#pragma once

#include <QMetaType>
#include <map>

#include "processors/ripesprocessor.h"

// Processors
#include "processors/ss/riscv_ss.h"

enum class ProcessorID { RISCV_SS, RISCV_5S_WF, RISCV_5S_WOF };

struct ProcessorDescription {
    ProcessorID id;
    QString ISA;
    QString name;
    QString description;
};

Q_DECLARE_METATYPE(ProcessorID);

class ProcessorRegistry {
public:
    static constexpr ProcessorID defaultProcessor = ProcessorID::RISCV_SS;

    using ProcessorMap = std::map<ProcessorID, ProcessorDescription>;
    static Ripes::RipesProcessor* getProcessor() { return instance().m_currentProcessor.get(); }
    static ProcessorID currentProcessorID() { return instance().m_currentProcessorID; }
    static const ProcessorMap& getAvailableProcessors() { return instance().m_descriptions; }

    /**
     * @brief selectProcessor
     * @param id
     * @returns true if current processor changed
     */
    static bool selectProcessor(ProcessorID id) {
        if (id == instance().m_currentProcessorID) {
            return false;
        }

        // Create the new processor...
        instance().m_currentProcessorID = id;
        instance().createProcessor();
        return true;
    }

private:
    void createProcessor() {
        switch (m_currentProcessorID) {
            case ProcessorID::RISCV_5S_WOF:
            case ProcessorID::RISCV_5S_WF:
            case ProcessorID::RISCV_SS:
                break;
                // m_currentProcessor = std::make_unique<vsrtl::RISCV::SingleCycleRISCV>();
        }
    }

    ProcessorRegistry() {
        // Initialize processors

        // RISC-V single cycle
        ProcessorDescription desc;
        desc.id = ProcessorID::RISCV_SS;
        desc.ISA = "RV32IM";
        desc.name = "RISC-V Single Cycle";
        desc.description = "A single cycle RISC-V processor";
        m_descriptions[desc.id] = desc;

        // RISC-V 5-Stage with forwarding
        desc.id = ProcessorID::RISCV_5S_WF;
        desc.ISA = "RV32IM";
        desc.name = "RISC-V 5-Stage with forwarding";
        desc.description = "A 5-Stage in-order RISC-V processor with hazard detection and forwarding.";
        m_descriptions[desc.id] = desc;

        // RISC-V 5-stage without forwarding
        desc.id = ProcessorID::RISCV_5S_WOF;
        desc.ISA = "RV32IM";
        desc.name = "RISC-V 5-Stage without forwarding";
        desc.description =
            "A 5-Stage in-order RISC-V processor with no hazard detection and forwarding. \n\nThe user is expected to "
            "resolve all data hazard through insertions of NOP instructions to correctly schedule the code.";
        m_descriptions[desc.id] = desc;

        createProcessor();
    }

    static ProcessorRegistry& instance() {
        static ProcessorRegistry pr;
        return pr;
    }

    ProcessorID m_currentProcessorID = defaultProcessor;
    std::unique_ptr<Ripes::RipesProcessor> m_currentProcessor;

    ProcessorMap m_descriptions;
};
