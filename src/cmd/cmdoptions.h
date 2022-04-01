#pragma once

#include "assembler/program.h"
#include "processorregistry.h"
#include <QCommandLineParser>

namespace Ripes {

struct CmdModeOptions {
  QString src;
  SourceType srcType;
  ProcessorID proc;
  QStringList isaExtensions;
  bool verbose;

  // telemetry
  bool cycles = false;
  bool cpi = false;
  bool ipc = false;
};

/// Adds Ripes command-line mode options to a parser.
void addCmdOptions(QCommandLineParser &parser);
/// Parses Ripes command-line mode options to a CmdModeOptions struct. Returns
/// true if options were parsed successfully.
bool parseCmdOptions(QCommandLineParser &parser, QString &errorMessage,
                     CmdModeOptions &options);

} // namespace Ripes
