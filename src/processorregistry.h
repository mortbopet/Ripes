#pragma once

#include <QMetaType>
#include <map>
#include <memory>

#include "processors/interface/ripesprocessor.h"

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
    virtual ~ProcInfoBase() = default;
    ProcessorID id;
    QString name;
    QString description;
    RegisterInitialization defaultRegisterVals;
    std::vector<Layout> layouts;
    virtual ProcessorISAInfo isaInfo() const = 0;
    virtual std::unique_ptr<RipesProcessor> construct(const QStringList& extensions) = 0;
};

template <typename T>
class ProcInfo : public ProcInfoBase {
public:
    using ProcInfoBase::ProcInfoBase;
    std::unique_ptr<RipesProcessor> construct(const QStringList& extensions) { return std::make_unique<T>(extensions); }
    // At this point we force the processor type T to implement a static function identifying its supported ISA.
    ProcessorISAInfo isaInfo() const { return T::supportsISA(); }
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

    ProcessorRegistry();

    static ProcessorRegistry& instance() {
        static ProcessorRegistry pr;
        return pr;
    }

    ProcessorMap m_descriptions;
};  // namespace Ripes
}  // namespace Ripes
