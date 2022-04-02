#pragma once

#include "clioptions.h"
#include <QObject>

namespace Ripes {

/// The CLIRunner class is used to run Ripes in CLI mode.
/// Based on a CLIModeOptions struct, it will run the appropriate combination
/// of source processing (assembler/compiler/...), processor model execution
/// as well as telemetry gathering and reporting.
class CLIRunner : public QObject {
  Q_OBJECT
public:
  CLIRunner(const CLIModeOptions &options);

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

  CLIModeOptions m_options;
};

} // namespace Ripes
