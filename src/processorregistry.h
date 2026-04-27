#pragma once

#include <QMetaEnum>
#include <QMetaType>
#include <QPointF>
#include <map>
#include <memory>
#include <optional>

#include "isa/rv32isainfo.h"
#include "isa/rv64isainfo.h"
#include "processors/interface/ripesprocessor.h"

namespace Ripes {
Q_NAMESPACE

template <typename T>
QString enumToString(T value) {
  int castValue = static_cast<int>(value);
  return QMetaEnum::fromType<T>().valueToKey(castValue);
}
template <>
inline QString enumToString<uint>(uint value) {
  return QString::number(value);
}


// =============================== Processors =================================
// The order of the ProcessorID enum defines the order of which the processors
// will appear in the processor selection dialog.
enum ProcessorID { RV_SS, RV_5MC, RV_5S, RV_6S_DUAL, NUM_PROCESSORS };
Q_ENUM_NS(ProcessorID); // Register with the metaobject system

// ------------------------------- Variations ---------------------------------
typedef uint VariationID;
namespace Variations {
Q_NAMESPACE

#define VARIATION_ENUM(name, ...) namespace name { Q_NAMESPACE; enum name : VariationID { __VA_ARGS__ }; Q_ENUM_NS(name) }

VARIATION_ENUM(RV_SS, 
  RV32I, /* 32-Bit Integer */
  RV64I, /* 64-Bit Integer */

  RV32F, /* 32-Bit Floating Point */
  RV64F  /* 64-Bit Floating Point */
)
VARIATION_ENUM(RV_5MC, 
  RV32I_1M, /* 32-Bit 1 Memory */
  RV32I_2M, /* 32-Bit 2 Memory */
  RV64I_1M, /* 64-Bit 1 Memory */
  RV64I_2M  /* 64-Bit 2 Memory */
)
VARIATION_ENUM(RV_5S, 
  // FU = with Forward unit
  // HU = with Hazard unit

  /* 32-Bit Integer Variations */
  RV32I_FU_HU,
  RV32I_FU,
  RV32I_HU,
  RV32I,

  /* 32-Bit Floating Point Variations */
  // RV32F_FU_HU,
  // RV32F_FU,
  // RV32F_HU,
  RV32F,

  /* 64-Bit Integer Variations */
  RV64I_FU_HU,
  RV64I_FU,
  RV64I_HU,
  RV64I,

  /* 64-Bit Floating Point Variations */
  // RV64F_FU_HU,
  // RV64F_FU,
  // RV64F_HU,
  RV64F
)
VARIATION_ENUM(RV_6S_DUAL,  RV32I, RV64I )

} // namespace Variations
// ============================================================================

//------------------------------------------------------------------------------
// General Info Interface
//------------------------------------------------------------------------------

using RegisterInitialization =
    std::map<std::string_view, std::map<unsigned, VInt>>;

struct Layout {
  QString name;
  QString file;
  /**
   * @brief stageLabelPositions
   * Stage labels are not a part of the VSRTL processor model, and as such are
   * not serialized within the models layout. The first value in the points
   * determines the position of stage labels as a relative distance based on the
   * processor models' width in the VSRTL view. Should be in the range [0;1].
   * The second value in the point determines the y-position of the label, as a
   * multiple of the height of the font used. This is used so that multiple
   * labels can be "stacked" over one another. Must contain an entry for each
   * stage in the processor model.
   */
  std::map<StageIndex, QPointF> stageLabelPositions;
  bool operator==(const Layout &rhs) const { return this->name == rhs.name; }
};

class VariationInfo {
public:
  using Options_t = std::unordered_set<QString>;

  // some default bit widths
  static constexpr uint BIT_WIDTH_8 = 8;
  static constexpr uint BIT_WIDTH_16 = 16;
  static constexpr uint BIT_WIDTH_32 = 32;
  static constexpr uint BIT_WIDTH_64 = 64;

  const VariationID id;
  const QString id_name;

  Options_t options;
  uint bitWidth;

  template <typename ID_t>
  VariationInfo(ID_t _id, const Options_t &_options, uint _bitWidth)
      : id(static_cast<VariationID>(_id)), id_name(enumToString<ID_t>(_id)),
        options(_options), bitWidth(_bitWidth) {}

  void addOption(const QString &option) { options.insert(option); }
  void addOptions(const Options_t &opts) {
    options.insert(opts.begin(), opts.end());
  }

  bool operator==(const VariationInfo &rhs) const {
    return match(rhs.options, rhs.bitWidth);
  }

  bool match(const Options_t &opts, uint width) const {
    return options == opts && bitWidth == width;
  }
};

class ProcVariationInfoBase {
public:
  ProcVariationInfoBase(const QString _name, 
                        const QString _desc,
                        const VariationInfo &_variationInfo,
                        const std::vector<Layout> &_layouts,
                        const RegisterInitialization &_defaultRegVals = {})
      : name(std::move(_name)), description(std::move(_desc)), 
        defaultRegisterVals(_defaultRegVals),
        layouts(_layouts), variationInfo(_variationInfo) {}
  virtual ~ProcVariationInfoBase() = default;

  QString name;
  QString description;
  RegisterInitialization defaultRegisterVals;
  std::vector<Layout> layouts;
  VariationInfo variationInfo;

  virtual const ProcessorISAInfo& isaInfo() const = 0;
  virtual std::unique_ptr<RipesProcessor>
  construct(const ExtensionSetInfo &extensions) = 0;
};

class ProcessorRegistry;
class ProcClassInfo {
public:
  using VariationMap =
      std::map<VariationID, std::shared_ptr<ProcVariationInfoBase>>;

  template<typename ID_t>
  ProcClassInfo(ProcessorID _id, ISA _isa, QString _name,
                ID_t _defaultVariationID)
      : id(_id), isa(_isa), name(std::move(_name)),
        defaultVariationID(static_cast<VariationID>(_defaultVariationID)) {}

  ProcessorID id;
  ISA isa;
  QString name;
  VariationID defaultVariationID;
  VariationMap variations;

  void addVariation(std::shared_ptr<ProcVariationInfoBase> variation) {
    verifyVariation(variation->variationInfo);
    variations.emplace(variation->variationInfo.id, std::move(variation));
  }

  /// Get all bit widths present in the variations of this processor class
  std::set<uint> getBitWidths() const {
    std::set<uint> bitWidths;
    for (const auto &varPair : variations) {
      bitWidths.insert(varPair.second->variationInfo.bitWidth);
    }
    return bitWidths;
  }

  /// Get all variation infos present in the variations of this processor class
  std::set<const VariationInfo *> getVariationInfos() const {
    std::set<const VariationInfo *> variationInfos;
    for (const auto &varPair : variations) {
      variationInfos.insert(&varPair.second->variationInfo);
    }
    return variationInfos;
  }

  /// Get all variations matching the given bit width
  std::set<const VariationInfo *> getVariationsByBitWidth(uint bitWidth) const {
    std::set<const VariationInfo *> filteredVariations;
    for (const auto &varPair : variations) {
      if (varPair.second->variationInfo.bitWidth == bitWidth) {
        filteredVariations.insert(&varPair.second->variationInfo);
      }
    }

    return filteredVariations;
  }

  /// Check if a VariationID exists in this processor class
  bool hasVariation(VariationID id) const { return variations.count(id) > 0; }
  /// Check if a VariationID exists in this processor class
  template <typename ID_t>
  bool hasVariation(ID_t id) const {
    return hasVariation(static_cast<VariationID>(id));
  }

  /// Get variation by its ID
  std::optional<const std::shared_ptr<ProcVariationInfoBase>>
  getVariation(VariationID id) const {
    auto it = variations.find(id);
    if (it != variations.end()) {
      return it->second;
    }
    return std::nullopt;
  }
  /// Get variation by its ID (templated version)
  template <typename ID_t>
  std::optional<const std::shared_ptr<ProcVariationInfoBase>>
  getVariation(ID_t id) const {
    return getVariation(static_cast<VariationID>(id));
  }

  /// Try getting a variation by its ID, else return the default variation if
  /// not found
  const std::shared_ptr<ProcVariationInfoBase>
  getVariationOrDefault(VariationID id) const {
    return getVariation(id).value_or(variations.at(defaultVariationID));
  }

  /// Try getting a variation by its ID (templated version), else return the
  /// default variation if not found
  template <typename ID_t>
  const std::shared_ptr<ProcVariationInfoBase>
  getVariationOrDefault(ID_t id) const {
    return getVariationOrDefault(static_cast<VariationID>(id));
  }

private:
  friend class ProcessorRegistry;
  void inline verifyVariation(const VariationInfo &info) const {
    // verify no duplicate variations
    Q_ASSERT(variations.count(info.id) == 0 &&
             "Duplicate variations not allowed.");
  }
  void inline finalVerify() const {
    // verify that the default variation exists
    Q_ASSERT(variations.count(defaultVariationID) > 0 &&
             "default variation of the processor class must exist.");
  }
};

//------------------------------------------------------------------------------
// Specification and Registry
//------------------------------------------------------------------------------

template <typename T>
class ProcVariationInfo : public ProcVariationInfoBase {
public:
  using ProcVariationInfoBase::ProcVariationInfoBase;
  std::unique_ptr<RipesProcessor> construct(const ExtensionSetInfo &extensions) override {
    return std::make_unique<T>(extensions);
  }
  // At this point we force the processor type T to implement a static function
  // identifying its supported ISA.
  const ProcessorISAInfo& isaInfo() const override { return T::supportsISA(); }
};

class ProcessorRegistry {
public:
  using ProcessorMap = std::map<ProcessorID, ProcClassInfo>;
  static const ProcessorMap &getAvailableProcessors() {
    return instance().m_descriptions;
  }

  static const ProcClassInfo &getProcessorClassInfo(ProcessorID id) {
    auto desc = instance().m_descriptions.find(id);
    Q_ASSERT(desc != instance().m_descriptions.end());
    return desc->second;
  }

  /* get a processor variation if given variation is not found returns default
   * variation */
  template <typename ID_t = VariationID>
  static const std::shared_ptr<ProcVariationInfoBase>
  getDescription(ProcessorID id, ID_t varID) {
    auto desc = instance().m_descriptions.find(id);
    if (desc == instance().m_descriptions.end()) {
      // processor ID not found, return first available processor's default
      // variation
      const auto &firstDesc = instance().m_descriptions.begin()->second;
      return firstDesc.getVariationOrDefault(firstDesc.defaultVariationID);
    }
    return desc->second.getVariationOrDefault(varID);
  }

  template <typename ID_t = VariationID>
  static std::unique_ptr<RipesProcessor>
  constructProcessor(ProcessorID id, ID_t varID,
                     const ExtensionSetInfo &extensions) {
    return getProcessorClassInfo(id).getVariationOrDefault(varID)->construct(
        extensions);
  }

private:
  void addProcessor(ProcClassInfo &&classInfo) {
    Q_ASSERT(m_descriptions.count(classInfo.id) == 0);
    classInfo.finalVerify();
    m_descriptions.emplace(classInfo.id, std::move(classInfo));
  }

  ProcessorRegistry();

  static ProcessorRegistry &instance() {
    static ProcessorRegistry pr;
    return pr;
  }

  ProcessorMap m_descriptions;
}; // namespace Ripes
} // namespace Ripes
