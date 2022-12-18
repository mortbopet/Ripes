#pragma once

#include "assembler/program.h"
#include "processorregistry.h"
#include "telemetry.h"
#include <QCommandLineParser>
#include <set>

namespace Ripes {

struct CLIModeOptions {
  QString src;
  SourceType srcType;
  ProcessorID proc;
  QStringList isaExtensions;
  bool verbose = false;
  QString outputFile = "";
  bool jsonOutput = false;
  int timeout = 0;
  RegisterInitialization regInit;

  // A list of enabled telemetry options.
  std::vector<std::shared_ptr<Telemetry>> telemetry;
};

/// Adds Ripes CLI options to a parser.
void addCLIOptions(QCommandLineParser &parser, Ripes::CLIModeOptions &options);

/// Parses Ripes CLI mode options to a CLIModeOptions struct. Returns
/// true if options were parsed successfully.
bool parseCLIOptions(QCommandLineParser &parser, QString &errorMessage,
                     CLIModeOptions &options);

} // namespace Ripes
