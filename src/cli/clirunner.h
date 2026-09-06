#pragma once

#include "clioptions.h"
#include <QObject>
#include <memory>

namespace Ripes {

class CacheSim;
class L1CacheShim;

/// The CLIRunner class is used to run Ripes in CLI mode.
/// Based on a CLIModeOptions struct, it will run the appropriate combination
/// of source processing (assembler/compiler/...), processor model execution
/// as well as telemetry gathering and reporting.
class CLIRunner : public QObject {
  Q_OBJECT
public:
  CLIRunner(const CLIModeOptions &options);
  ~CLIRunner();

  /// Runs the CLI mode.
  int run();

private:
  /// Process the provided source file (assembling, compiling, loading, ...)
  int processInput();

  /// Runs the processor model until the program is finished.
  int runModel();

  /// Prints requested telemetry to the console/output file.
  int postRun();
  void info(QString msg, bool alwaysPrint = false, bool header = false,
            const QString &prefix = "INFO");
  void error(const QString &msg);

  /// Instantiates L1 instruction/data cache simulators and wires them into the
  /// processor, mirroring the GUI cache tab. Used when --cache is requested.
  void setupCaches();

  CLIModeOptions m_options;

  // L1 cache simulation state (only populated when --cache is set). The shims
  // connect to ProcessorHandler::processorClocked and drive the cache sims in
  // lockstep with the processor.
  std::unique_ptr<L1CacheShim> m_l1iShim;
  std::unique_ptr<L1CacheShim> m_l1dShim;
  std::shared_ptr<CacheSim> m_l1iCache;
  std::shared_ptr<CacheSim> m_l1dCache;
};

} // namespace Ripes
