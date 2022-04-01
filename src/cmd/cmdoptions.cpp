#include "cmdoptions.h"
#include "processorregistry.h"
#include "telemetry.h"
#include <QFile>
#include <QMetaEnum>

namespace Ripes {

template <typename T>
QString enumToString(T value) {
  int castValue = static_cast<int>(value);
  return QMetaEnum::fromType<T>().valueToKey(castValue);
}

void addCmdOptions(QCommandLineParser &parser, Ripes::CmdModeOptions &options) {

  QCommandLineOption srcOption("src", "Source file", "src");
  parser.addOption(srcOption);

  QCommandLineOption srcTypeOption("t", "Source type. Options: [c, asm, bin]",
                                   "type", "asm");
  srcTypeOption.setDefaultValue("asm");
  parser.addOption(srcTypeOption);

  // Processor models. Generate information from processor registry.
  QStringList processorOptions;
  for (int i = 0; i < ProcessorID::NUM_PROCESSORS; i++)
    processorOptions.push_back(
        enumToString<ProcessorID>(static_cast<ProcessorID>(i)));
  QString desc =
      "Processor model. Options: [" + processorOptions.join(", ") + "]";
  QCommandLineOption processorOption("proc", desc, "proc");
  parser.addOption(processorOption);

  // ISA extensions.
  QCommandLineOption isaExtensionsOptions("isaexts",
                                          "ISA extensions to enable (comma "
                                          "separated)",
                                          "isaexts", "");
  parser.addOption(isaExtensionsOptions);

  QCommandLineOption verboseOption("v", "Verbose output");
  parser.addOption(verboseOption);

  // Output to file
  QCommandLineOption outputOption(
      "output",
      "Telemetry output file. If not set, telemtry is printed to stdout.",
      "output", "");
  parser.addOption(outputOption);

  // Telemtry reporting
  options.telemetry.push_back(std::make_shared<CyclesTelemetry>());
  options.telemetry.push_back(std::make_shared<CPITelemetry>());
  options.telemetry.push_back(std::make_shared<IPCTelemetry>());

  for (auto &telemetry : options.telemetry) {
    QCommandLineOption telemetryOption(telemetry->key(),
                                       telemetry->description());
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
    if (parser.isSet(telemetry->key()))
      telemetry->enable();

  return true;
}

} // namespace Ripes
