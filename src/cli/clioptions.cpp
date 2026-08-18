#include "clioptions.h"
#include "processorregistry.h"
#include "radix.h"
#include "ripessettings.h"
#include "telemetry.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>

#include <sstream>
#include <string>

#include "STLExtras.h"

namespace Ripes {

// String tokens accepted (case-insensitively) for the cache policy fields of a
// --cache-config JSON spec. Kept terse and CLI-friendly rather than reusing the
// GUI's display strings.
static const std::map<QString, WritePolicy> s_writePolicyTokens{
    {"writeback", WritePolicy::WriteBack},
    {"writethrough", WritePolicy::WriteThrough}};
static const std::map<QString, WriteAllocPolicy> s_writeAllocTokens{
    {"writeallocate", WriteAllocPolicy::WriteAllocate},
    {"nowriteallocate", WriteAllocPolicy::NoWriteAllocate}};
static const std::map<QString, ReplPolicy> s_replPolicyTokens{
    {"lru", ReplPolicy::LRU}, {"random", ReplPolicy::Random}};

template <typename T>
static QString tokenKeys(const std::map<QString, T> &m) {
  QStringList keys;
  for (const auto &kv : m)
    keys.push_back(kv.first);
  return keys.join(", ");
}

// Returns the list of cache presets available (built-in + user-defined),
// as stored in the Ripes settings.
static QList<CachePreset> availableCachePresets() {
  return RipesSettings::value(RIPES_SETTING_CACHE_PRESETS)
      .value<QList<CachePreset>>();
}

// Parses a single cache spec (blocks/lines/ways + policies) from a JSON object.
// The three geometry fields are required; the three policy fields are optional
// and default to a write-back/write-allocate/LRU cache. Returns false and sets
// 'errorMessage' on any error.
static bool parseCacheSpec(const QString &cacheName, const QJsonObject &obj,
                           CachePreset &out, QString &errorMessage) {
  const auto requireInt = [&](const QString &key, int &dst) -> bool {
    if (!obj.contains(key) || !obj.value(key).isDouble()) {
      errorMessage = "Cache spec '" + cacheName +
                     "' is missing required "
                     "integer field '" +
                     key + "' (--cache-config).";
      return false;
    }
    dst = obj.value(key).toInt();
    return true;
  };

  if (!requireInt("blocks", out.blocks) || !requireInt("lines", out.lines) ||
      !requireInt("ways", out.ways))
    return false;

  // Optional policy fields (defaults match the direct-mapped preset).
  out.wrPolicy = WritePolicy::WriteBack;
  out.wrAllocPolicy = WriteAllocPolicy::WriteAllocate;
  out.replPolicy = ReplPolicy::LRU;

  const auto parsePolicy = [&](const QString &key, const auto &tokens,
                               auto &dst) -> bool {
    if (!obj.contains(key))
      return true; // optional
    const QString tok = obj.value(key).toString().toLower();
    const auto it = tokens.find(tok);
    if (it == tokens.end()) {
      errorMessage = "Cache spec '" + cacheName + "' has invalid '" + key +
                     "' value '" + obj.value(key).toString() +
                     "'. Valid values: " + tokenKeys(tokens) +
                     " (--cache-config).";
      return false;
    }
    dst = it->second;
    return true;
  };

  if (!parsePolicy("writePolicy", s_writePolicyTokens, out.wrPolicy) ||
      !parsePolicy("writeAllocatePolicy", s_writeAllocTokens,
                   out.wrAllocPolicy) ||
      !parsePolicy("replacementPolicy", s_replPolicyTokens, out.replPolicy))
    return false;

  out.name = cacheName;
  return true;
}

void addCLIOptions(QCommandLineParser &parser, Ripes::CLIModeOptions &options) {
  parser.addOption(QCommandLineOption("src", "Path to source file.", "path"));
  parser.addOption(QCommandLineOption(
      "t", "Source file type. Options: [c, asm, bin, elf]", "type", "asm"));

  // Processor models. Generate information from processor registry.
  QStringList processorOptions;
  for (int i = 0; i < ProcessorID::NUM_PROCESSORS; i++)
    processorOptions.push_back(
        enumToString<ProcessorID>(static_cast<ProcessorID>(i)));
  QString desc =
      "Processor model. Options: [" + processorOptions.join(", ") + "]";
  parser.addOption(QCommandLineOption("proc", desc, "name"));
  parser.addOption(QCommandLineOption("isaexts",
                                      "ISA extensions to enable (comma "
                                      "separated)",
                                      "extensions", ""));
  parser.addOption(QCommandLineOption(
      "reginit",
      "Comma-separated list of register initialization values. The register "
      "value may be specified in signed, hex, or boolean notation. Can be used "
      "multiple times to initialize more than one register file type. Format:\n"
      "<register file>:<register idx>=<value>,<register idx>=<value>",
      "regfile:[rid=v]"));
  parser.addOption(QCommandLineOption(
      "timeout",
      "Simulation timeout in milliseconds. If simulation does not finish "
      "within the specified time, it will be aborted.",
      "ms", "0"));
  parser.addOption(QCommandLineOption("v", "Verbose output"));
  parser.addOption(QCommandLineOption(
      "output", "Report output file. If not set, report is printed to stdout.",
      "path"));
  parser.addOption(QCommandLineOption("json", "JSON-formatted report."));

  parser.addOption(QCommandLineOption("all", "Enable all report options."));

  // telemetry reporting
  options.telemetry.push_back(std::make_shared<CyclesTelemetry>());
  options.telemetry.push_back(std::make_shared<InstrsRetiredTelemetry>());
  options.telemetry.push_back(std::make_shared<CPITelemetry>());
  options.telemetry.push_back(std::make_shared<IPCTelemetry>());
  options.telemetry.push_back(std::make_shared<PipelineTelemetry>());
  options.telemetry.push_back(std::make_shared<RegisterTelemetry>());
  options.telemetry.push_back(std::make_shared<ExecutionTimeTelemetry>());
  options.telemetry.push_back(std::make_shared<RunInfoTelemetry>(&parser));

  for (auto &telemetry : options.telemetry) {
    QString desc = "Report " + telemetry->description();
    QCommandLineOption telemetryOption(telemetry->key(), desc);
    parser.addOption(telemetryOption);
  }

  // Cache simulation. Rather than a bare on/off flag, cache simulation is
  // configured either from a named preset (as in the GUI) or from a JSON spec
  // describing the L1I and/or L1D caches. Enabling either option activates the
  // cache statistics report; hence the CacheTelemetry is registered here
  // (after the generic loop, so it does not get a bare --cache option) and
  // enabled during parsing when a cache is configured.
  parser.addOption(QCommandLineOption(
      "cache-preset",
      "Enable L1 instruction & data cache simulation using a named preset (as "
      "listed in the GUI cache tab). Applies the preset to both caches.",
      "name"));
  parser.addOption(QCommandLineOption(
      "cache-config",
      "Enable L1 cache simulation from a JSON spec file. The document may "
      "contain \"L1I\" and/or \"L1D\" objects; each object requires integer "
      "fields \"blocks\", \"lines\" and \"ways\" and optionally "
      "\"writePolicy\" (writeback|writethrough), \"writeAllocatePolicy\" "
      "(writeallocate|nowriteallocate) and \"replacementPolicy\" (lru|random). "
      "A cache is simulated only if its object is present.",
      "path"));
  options.telemetry.push_back(std::make_shared<CacheTelemetry>());
}

bool parseCLIOptions(QCommandLineParser &parser, QString &errorMessage,
                     CLIModeOptions &options) {
  options.verbose = parser.isSet("v");

  if (!parser.isSet("src")) {
    errorMessage = "No source file specified (--src)";
    return false;
  }
  options.src = parser.value("src");

  if (!parser.isSet("t")) {
    errorMessage = "No source type specified (--t)";
    return false;
  }

  if (parser.value("t") == "c") {
    options.srcType = SourceType::C;
  } else if (parser.value("t") == "asm") {
    options.srcType = SourceType::Assembly;
  } else if (parser.value("t") == "bin") {
    options.srcType = SourceType::FlatBinary;
  } else if (parser.value("t") == "elf") {
    options.srcType = SourceType::ExternalELF;
  } else {
    errorMessage = "Invalid source type (--t)";
    return false;
  }

  if (!parser.isSet("proc")) {
    errorMessage = "No processor specified (-proc).";
    return false;
  }
  bool ok;
  int procID = QMetaEnum::fromType<ProcessorID>().keyToValue(
      parser.value("proc").toStdString().c_str(), &ok);
  if (!ok) {
    errorMessage = "Invalid processor model specified '" +
                   parser.value("proc") + "' (--proc).";
    return false;
  }
  options.proc = static_cast<ProcessorID>(procID);

  options.jsonOutput = parser.isSet("json");

  if (parser.isSet("isaexts")) {
    options.isaExtensions = parser.value("isaexts").split(",");

    // Validate the ISA extensions with respect to the selected processor.
    auto exts = ProcessorRegistry::getDescription(options.proc)
                    .isaInfo()
                    .supportedExtensions;

    for (auto &ext : std::as_const(options.isaExtensions)) {
      if (!exts.contains(ext)) {
        errorMessage =
            "Invalid ISA extension '" + ext + "' specified (--isaexts).";
        errorMessage +=
            " Processor '" + enumToString<ProcessorID>(options.proc) + "'";
        errorMessage += " supports extensions: " + exts.join(", ");
        return false;
      }
    }
  }

  if (parser.isSet("timeout")) {
    bool ok;
    options.timeout = parser.value("timeout").toUInt(&ok);
    if (!ok) {
      errorMessage = "Invalid timeout value specified (--timeout).";
      return false;
    }
  }

  options.outputFile = parser.value("output");

  // Configure L1 cache simulation from either a named preset or a JSON spec.
  if (parser.isSet("cache-preset") && parser.isSet("cache-config")) {
    errorMessage = "Options --cache-preset and --cache-config are mutually "
                   "exclusive.";
    return false;
  }
  if (parser.isSet("cache-preset")) {
    const QString name = parser.value("cache-preset");
    const auto presets = availableCachePresets();
    const auto it =
        std::find_if(presets.begin(), presets.end(),
                     [&](const CachePreset &p) { return p.name == name; });
    if (it == presets.end()) {
      QStringList names;
      for (const auto &p : presets)
        names.push_back("\"" + p.name + "\"");
      errorMessage =
          "Unknown cache preset '" + name +
          "' (--cache-preset). Available presets: " + names.join(", ") + ".";
      return false;
    }
    // Apply the same preset to both the instruction and data cache.
    options.l1iCache = *it;
    options.l1dCache = *it;
  } else if (parser.isSet("cache-config")) {
    const QString path = parser.value("cache-config");
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      errorMessage =
          "Could not open cache config file '" + path + "' (--cache-config).";
      return false;
    }
    QJsonParseError jsonErr;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &jsonErr);
    if (doc.isNull() || !doc.isObject()) {
      errorMessage = "Invalid cache config JSON in '" + path +
                     "': " + jsonErr.errorString() + " (--cache-config).";
      return false;
    }
    const QJsonObject root = doc.object();
    if (!root.contains("L1I") && !root.contains("L1D")) {
      errorMessage = "Cache config '" + path +
                     "' must contain at least one of "
                     "\"L1I\" or \"L1D\" (--cache-config).";
      return false;
    }
    if (root.contains("L1I")) {
      CachePreset spec;
      if (!parseCacheSpec("L1I", root.value("L1I").toObject(), spec,
                          errorMessage))
        return false;
      options.l1iCache = spec;
    }
    if (root.contains("L1D")) {
      CachePreset spec;
      if (!parseCacheSpec("L1D", root.value("L1D").toObject(), spec,
                          errorMessage))
        return false;
      options.l1dCache = spec;
    }
  }

  // Validate register initializations
  if (parser.isSet("reginit")) {
    const auto &procisa =
        ProcessorRegistry::getAvailableProcessors().at(options.proc)->isaInfo();
    const auto *isa = procisa.isa.get();
    for (const auto &regFileInit : parser.values("reginit")) {
      if (!regFileInit.contains(':')) {
        errorMessage = "Cannot find register file type (--reginit).";
        return false;
      }
      auto regFileSplit = regFileInit.indexOf(':');
      QString regFile = regFileInit.mid(0, regFileSplit);

      QStringList regInitList = regFileInit.mid(regFileSplit + 1).split(",");
      for (auto &regInit : regInitList) {
        QStringList regInitParts = regInit.split("=");
        if (regInitParts.size() != 2) {
          errorMessage = "Invalid register initialization '" + regInit +
                         "' specified (--reginit).";
          return false;
        }
        bool ok;
        int regIdx = regInitParts[0].toInt(&ok);
        if (!ok) {
          errorMessage = "Invalid register index '" + regInitParts[0] +
                         "' specified (--reginit).";
          return false;
        }

        auto &vstr = regInitParts[1];
        VInt regVal = decodeRadixValue(vstr, &ok);

        if (!ok) {
          errorMessage =
              "Invalid register value '" + vstr + "' specified (--reginit).";
          return false;
        }

        std::string_view rfid = "";
        auto fileNames = isa->regFileNames();
        for (const auto &regFileName : fileNames) {
          if (regFile == QString(regFileName.data())) {
            rfid = regFileName;
            break;
          }
        }
        if (rfid.empty()) {
          errorMessage = "Invalid register file type '" + regFile +
                         "' specified (--reginit). Valid types for '" +
                         parser.value("proc") + "' with extensions [";
          std::stringstream extInfo;
          std::string isaExtensions =
              options.isaExtensions.join("").toStdString();
          llvm::interleaveComma(isaExtensions, extInfo);
          extInfo << "]: [";
          llvm::interleaveComma(fileNames, extInfo);
          extInfo << "]";
          errorMessage += extInfo.str();
          return false;
        }

        if (options.regInit.count(rfid) == 0) {
          options.regInit[rfid] = {{regIdx, regVal}};
        } else {
          if (options.regInit.at(rfid).count(regIdx) > 0) {
            errorMessage = "Duplicate register initialization for register " +
                           QString::number(regIdx) + " specified (--reginit).";
            return false;
          }

          options.regInit[rfid][regIdx] = regVal;
        }
      }
    }
  }

  // Enable selected telemetry options. The cache telemetry is special-cased:
  // it is only meaningful when a cache has actually been configured, so it is
  // not driven by --all or a bare key, but enabled below when a cache spec is
  // present.
  const bool cacheConfigured = options.l1iCache || options.l1dCache;
  for (auto &telemetry : options.telemetry) {
    if (dynamic_cast<CacheTelemetry *>(telemetry.get())) {
      if (cacheConfigured)
        telemetry->enable();
      continue;
    }
    if (parser.isSet("all") || parser.isSet(telemetry->key()))
      telemetry->enable();
  }

  return true;
}

} // namespace Ripes
