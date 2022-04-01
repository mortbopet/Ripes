#include "cmdoptions.h"
#include "processorregistry.h"
#include "telemetry.h"
#include <QFile>
#include <QMetaEnum>

namespace Ripes {

void addCmdOptions(QCommandLineParser &parser, Ripes::CmdModeOptions &options) {
  parser.addOption(QCommandLineOption("src", "Source file", "src"));
  parser.addOption(QCommandLineOption(
      "t", "Source type. Options: [c, asm, bin]", "type", "asm"));

  // Processor models. Generate information from processor registry.
  QStringList processorOptions;
  for (int i = 0; i < ProcessorID::NUM_PROCESSORS; i++)
    processorOptions.push_back(
        enumToString<ProcessorID>(static_cast<ProcessorID>(i)));
  QString desc =
      "Processor model. Options: [" + processorOptions.join(", ") + "]";
  parser.addOption(QCommandLineOption("proc", desc, "proc"));
  parser.addOption(QCommandLineOption("isaexts",
                                      "ISA extensions to enable (comma "
                                      "separated)",
                                      "isaexts", ""));
  parser.addOption(QCommandLineOption("v", "Verbose output"));
  parser.addOption(QCommandLineOption(
      "output",
      "Telemetry output file. If not set, telemetry is printed to stdout.",
      "output"));
  parser.addOption(
      QCommandLineOption("json", "Report JSON-formatted telemetry."));

  parser.addOption(QCommandLineOption("all", "Enable all telemetry options."));

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

bool parseCmdOptions(QCommandLineParser &parser, QString &errorMessage,
                     CmdModeOptions &options) {
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

    for (auto &ext : qAsConst(options.isaExtensions)) {
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

  options.outputFile = parser.value("output");

  // Enable selected telemetry options.
  for (auto &telemetry : options.telemetry)
    if (parser.isSet("all") || parser.isSet(telemetry->key()))
      telemetry->enable();

  return true;
}

} // namespace Ripes
