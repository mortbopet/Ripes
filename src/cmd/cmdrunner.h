#pragma once

#include "cmdoptions.h"
#include <QObject>

namespace Ripes {

/// The CmdRunner class is used to run Ripes in command-line mode.
/// Based on a CmdModeOptions struct, it will run the appropriate combination
/// of source processing (assembler/compiler/...), processor model execution
/// as well as telemetry gathering and reporting.
class CmdRunner : public QObject {
  Q_OBJECT
public:
  CmdRunner(const CmdModeOptions &options);

  /// Runs the command-line mode.
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

  CmdModeOptions m_options;
};

} // namespace Ripes
