#pragma once

#include "assembler/program.h"
#include "processorregistry.h"
#include "telemetry.h"
#include <QCommandLineParser>
#include <set>

namespace Ripes {

struct CmdModeOptions {
  QString src;
  SourceType srcType;
  ProcessorID proc;
  QStringList isaExtensions;
  bool verbose;
  QString outputFile;
  bool jsonOutput;
  int timeout;

  // A list of enabled telemetry options.
  std::vector<std::shared_ptr<Telemetry>> telemetry;
};

/// Adds Ripes command-line mode options to a parser.
void addCmdOptions(QCommandLineParser &parser, Ripes::CmdModeOptions &options);
/// Parses Ripes command-line mode options to a CmdModeOptions struct. Returns
/// true if options were parsed successfully.
bool parseCmdOptions(QCommandLineParser &parser, QString &errorMessage,
                     CmdModeOptions &options);

} // namespace Ripes
