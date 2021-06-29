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

#ifdef RIPES_BUILD_VERILATOR_PROCESSORS
#include "processors/PicoRV32/ripes_picorv32.h"
#endif

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

#ifdef RIPES_BUILD_VERILATOR_PROCESSORS
    PICORV32,
#endif

    NUM_PROCESSORS
};
Q_ENUM_NS(Ripes::ProcessorID);  // Register with the metaobject system
// ============================================================================

using RegisterInitialization = std::map<unsigned, VInt>;
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

class ProcInfoBase {
public:
    ProcInfoBase(ProcessorID _id, const QString& _name, const QString& _desc, const std::vector<Layout>& _layouts,
                 const RegisterInitialization& _defaultRegVals = {})
        : id(_id), name(_name), description(_desc), defaultRegisterVals(_defaultRegVals), layouts(_layouts) {}
    ProcessorID id;
    QString name;
    QString description;
    RegisterInitialization defaultRegisterVals;
    std::vector<Layout> layouts;
    virtual const ISAInfoBase* isa() const = 0;
    virtual std::unique_ptr<RipesProcessor> construct(const QStringList& extensions) = 0;
};

template <typename T>
class ProcInfo : public ProcInfoBase {
public:
    using ProcInfoBase::ProcInfoBase;
    std::unique_ptr<RipesProcessor> construct(const QStringList& extensions) { return std::make_unique<T>(extensions); }
    // At this point we force the processor type T to implement a static function identifying its supported ISA.
    const ISAInfoBase* isa() const { return T::supportsISA(); }
};

class ProcessorRegistry {
public:
    using ProcessorMap = std::map<ProcessorID, std::unique_ptr<ProcInfoBase>>;
    static const ProcessorMap& getAvailableProcessors() { return instance().m_descriptions; }
    static const ProcInfoBase& getDescription(ProcessorID id) {
        auto desc = instance().m_descriptions.find(id);
        if (desc == instance().m_descriptions.end()) {
            return *instance().m_descriptions.begin()->second;
        }
        return *desc->second;
    }
    static std::unique_ptr<RipesProcessor> constructProcessor(ProcessorID id, const QStringList& extensions) {
        auto& _this = instance();
        auto it = _this.m_descriptions.find(id);
        Q_ASSERT(it != _this.m_descriptions.end());
        return it->second->construct(extensions);
    }

private:
    template <typename T>
    void addProcessor(const ProcInfo<T>& pinfo) {
        Q_ASSERT(m_descriptions.count(pinfo.id) == 0);
        m_descriptions[pinfo.id] = std::make_unique<ProcInfo<T>>(pinfo);
    }

    ProcessorRegistry() {
        // Initialize processors
        std::vector<Layout> layouts;
        RegisterInitialization defRegVals;

        // RISC-V single cycle
        layouts = {{"Standard", ":/layouts/RISC-V/rvss/rv_ss_standard_layout.json", {QPointF{0.5, 0}}},
                   {"Extended", ":/layouts/RISC-V/rvss/rv_ss_extended_layout.json", {QPointF{0.5, 0}}}};
        defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        addProcessor(ProcInfo<vsrtl::core::RVSS<uint32_t>>(ProcessorID::RV32_SS, "Single-cycle processor",
                                                           "A single cycle processor", layouts, defRegVals));
        addProcessor(ProcInfo<vsrtl::core::RVSS<uint64_t>>(ProcessorID::RV64_SS, "Single-cycle processor",
                                                           "A single cycle processor", layouts, defRegVals));

        // RISC-V 5-stage without forwarding or hazard detection
        layouts = {
            {"Standard",
             ":/layouts/RISC-V/rv5s_no_fw_hz/rv5s_no_fw_hz_standard_layout.json",
             {QPointF{0.08, 0}, QPointF{0.3, 0}, QPointF{0.54, 0}, QPointF{0.73, 0}, QPointF{0.88, 0}}},
            {"Extended",
             ":/layouts/RISC-V/rv5s_no_fw_hz/rv5s_no_fw_hz_extended_layout.json",
             {QPointF{0.08, 0.0}, QPointF{0.31, 0.0}, QPointF{0.56, 0.0}, QPointF{0.76, 0.0}, QPointF{0.9, 0.0}}}};
        defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        addProcessor(ProcInfo<vsrtl::core::RV5S_NO_FW_HZ<uint32_t>>(
            ProcessorID::RV32_5S_NO_FW_HZ, "5-stage processor w/o forwarding or hazard detection",
            "A 5-stage in-order processor with no forwarding or hazard detection/elimination.", layouts, defRegVals));
        addProcessor(ProcInfo<vsrtl::core::RV5S_NO_FW_HZ<uint64_t>>(
            ProcessorID::RV64_5S_NO_FW_HZ, "5-stage processor w/o forwarding or hazard detection",
            "A 5-stage in-order processor with no forwarding or hazard detection/elimination.", layouts, defRegVals));

        // RISC-V 5-stage without hazard detection
        layouts = {{"Standard",
                    ":/layouts/RISC-V/rv5s_no_hz/rv5s_no_hz_standard_layout.json",
                    {QPointF{0.08, 0}, QPointF{0.3, 0}, QPointF{0.53, 0}, QPointF{0.75, 0}, QPointF{0.88, 0}}},
                   {"Extended",
                    ":/layouts/RISC-V/rv5s_no_hz/rv5s_no_hz_extended_layout.json",
                    {QPointF{0.08, 0}, QPointF{0.28, 0}, QPointF{0.53, 0}, QPointF{0.78, 0}, QPointF{0.9, 0}}}};
        defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        addProcessor(ProcInfo<vsrtl::core::RV5S_NO_HZ<uint32_t>>(
            ProcessorID::RV32_5S_NO_HZ, "5-stage processor w/o hazard detection",
            "A 5-stage in-order processor with forwarding but no hazard detection/elimination.", layouts, defRegVals));
        addProcessor(ProcInfo<vsrtl::core::RV5S_NO_HZ<uint64_t>>(
            ProcessorID::RV64_5S_NO_HZ, "5-stage processor w/o hazard detection",
            "A 5-stage in-order processor with forwarding but no hazard detection/elimination.", layouts, defRegVals));

        // RISC-V 5-stage without forwarding unit
        layouts = {{"Standard",
                    ":/layouts/RISC-V/rv5s_no_fw/rv5s_no_fw_standard_layout.json",
                    {QPointF{0.08, 0}, QPointF{0.3, 0}, QPointF{0.53, 0}, QPointF{0.75, 0}, QPointF{0.88, 0}}},
                   {"Extended",
                    ":/layouts/RISC-V/rv5s_no_fw/rv5s_no_fw_extended_layout.json",
                    {QPointF{0.08, 0}, QPointF{0.28, 0}, QPointF{0.53, 0}, QPointF{0.78, 0}, QPointF{0.9, 0}}}};
        defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        addProcessor(ProcInfo<vsrtl::core::RV5S_NO_FW<uint32_t>>(
            ProcessorID::RV32_5S_NO_FW, "5-Stage processor w/o forwarding unit",
            "A 5-stage in-order processor with hazard detection/elimination but no forwarding unit.", layouts,
            defRegVals));
        addProcessor(ProcInfo<vsrtl::core::RV5S_NO_FW<uint64_t>>(
            ProcessorID::RV64_5S_NO_FW, "5-Stage processor w/o forwarding unit",
            "A 5-stage in-order processor with hazard detection/elimination but no forwarding unit.", layouts,
            defRegVals));

        // RISC-V 5-stage
        layouts = {{"Standard",
                    ":/layouts/RISC-V/rv5s/rv5s_standard_layout.json",
                    {QPointF{0.08, 0}, QPointF{0.29, 0}, QPointF{0.55, 0}, QPointF{0.75, 0}, QPointF{0.87, 0}}},
                   {"Extended",
                    ":/layouts/RISC-V/rv5s/rv5s_extended_layout.json",
                    {QPointF{0.08, 0}, QPointF{0.28, 0}, QPointF{0.54, 0}, QPointF{0.78, 0}, QPointF{0.9, 0}}}};
        defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        addProcessor(ProcInfo<vsrtl::core::RV5S<uint32_t>>(
            ProcessorID::RV32_5S, "5-stage processor",
            "A 5-stage in-order processor with hazard detection/elimination and forwarding.", layouts, defRegVals));
        addProcessor(ProcInfo<vsrtl::core::RV5S<uint64_t>>(
            ProcessorID::RV64_5S, "5-stage processor",
            "A 5-stage in-order processor with hazard detection/elimination and forwarding.", layouts, defRegVals));

        // RISC-V 6-stage dual issue
        layouts = {{"Extended",
                    ":/layouts/RISC-V/rv6s_dual/rv6s_dual_extended_layout.json",
                    {{QPointF{0.06, 0}, QPointF{0.06, 1}, QPointF{0.22, 0}, QPointF{0.22, 1}, QPointF{0.35, 0},
                      QPointF{0.35, 1}, QPointF{0.54, 0}, QPointF{0.54, 1}, QPointF{0.78, 0}, QPointF{0.78, 1},
                      QPointF{0.87, 0}, QPointF{0.87, 1}}}}};
        defRegVals = {{2, 0x7ffffff0}, {3, 0x10000000}};
        addProcessor(ProcInfo<vsrtl::core::RV6S_DUAL<uint32_t>>(
            ProcessorID::RV32_6S_DUAL, "6-stage dual-issue processor",
            "A 6-stage dual-issue in-order processor. Each way may execute arithmetic instructions, whereas way 1 "
            "is reserved for controlflow and ecall instructions, and way 2 for memory accessing instructions.",
            layouts, defRegVals));
        addProcessor(ProcInfo<vsrtl::core::RV6S_DUAL<uint64_t>>(
            ProcessorID::RV64_6S_DUAL, "6-stage dual-issue processor",
            "A 6-stage dual-issue in-order processor. Each way may execute arithmetic instructions, whereas way 1 "
            "is reserved for controlflow and ecall instructions, and way 2 for memory accessing instructions.",
            layouts, defRegVals));

#ifdef RIPES_BUILD_VERILATOR_PROCESSORS
        addProcessor(ProcInfo<PicoRV32>(ProcessorID::PICORV32, "PicoRV32", "PicoRV32", {}, defRegVals));
#endif
    }

    static ProcessorRegistry& instance() {
        static ProcessorRegistry pr;
        return pr;
    }

    ProcessorMap m_descriptions;
};  // namespace Ripes
}  // namespace Ripes
