#include "clioptions.h"
#include "processorregistry.h"
#include "radix.h"
#include "telemetry.h"
#include <QFile>
#include <QMetaEnum>

namespace Ripes {

void addCLIOptions(QCommandLineParser &parser, Ripes::CLIModeOptions &options) {
  parser.addOption(QCommandLineOption("src", "Path to source file.", "path"));
  parser.addOption(QCommandLineOption(
      "t", "Source file type. Options: [c, asm, bin]", "type", "asm"));

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
      "value may be specified in signed, hex, or boolean notation. Format:\n"
      "<register idx>=<value>,<register idx>=<value>",
      "[rid:v]"));
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
  options.telemetry.push_back(std::make_shared<RunInfoTelemetry>(&parser));

  for (auto &telemetry : options.telemetry) {
    QString desc = "Report " + telemetry->description();
    QCommandLineOption telemetryOption(telemetry->key(), desc);
    parser.addOption(telemetryOption);
  }
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

  // Validate register initializations
  if (parser.isSet("reginit")) {
    QStringList regInitList = parser.value("reginit").split(",");
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
      VInt regVal;
      if (vstr.startsWith("0x"))
        regVal = decodeRadixValue(vstr, Radix::Hex, &ok);
      else if (vstr.startsWith("0b"))
        regVal = decodeRadixValue(vstr, Radix::Binary, &ok);
      else
        regVal = decodeRadixValue(vstr, Radix::Signed, &ok);

      if (!ok) {
        errorMessage =
            "Invalid register value '" + vstr + "' specified (--reginit).";
        return false;
      }

      if (options.regInit.count(regIdx) > 0) {
        errorMessage = "Duplicate register initialization for register " +
                       QString::number(regIdx) + " specified (--reginit).";
        return false;
      }

      options.regInit[regIdx] = regVal;
    }
  }

  // Enable selected telemetry options.
  for (auto &telemetry : options.telemetry)
    if (parser.isSet("all") || parser.isSet(telemetry->key()))
      telemetry->enable();

  return true;
}

} // namespace Ripes
