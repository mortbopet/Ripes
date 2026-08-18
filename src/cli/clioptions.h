#pragma once

#include "assembler/program.h"
#include "cachesim/cachesim.h"
#include "processorregistry.h"
#include "telemetry.h"
#include <QCommandLineParser>
#include <optional>
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

  // Optional L1 instruction/data cache configurations. When set, the
  // corresponding cache is simulated during the run (mirroring the GUI cache
  // tab) so cache behaviour/overhead can be exercised and reported headlessly.
  // Populated from either a named preset (--cache-preset) or a JSON spec
  // (--cache-config).
  std::optional<CachePreset> l1iCache;
  std::optional<CachePreset> l1dCache;

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
