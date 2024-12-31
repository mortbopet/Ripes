#include "clirunner.h"
#include "ccmanager.h"
#include "io/iomanager.h"
#include "loaddialog.h"
#include "processorhandler.h"
#include "programutilities.h"
#include "syscall/systemio.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace Ripes {

/**
 * An extended QVariant-to-string convertion method which handles a special
 * cases such as QVariantMap and QStringList.
 *
 * @param v The QVariant to convert to a string.
 * @returns A string representation of the QVariant.
 */
static QString qVariantToString(QVariant &v) {
  QString def = v.toString();
  if (!def.isEmpty())
    return def;

  if (v.canConvert<QVariantMap>()) {
    QVariantMap map = qvariant_cast<QVariantMap>(v);
    QString out;
    for (auto it : map.toStdMap())
      out += it.first + ": " + qVariantToString(it.second) + "\n";
    return out;
  } else if (v.canConvert<QStringList>()) {
    QStringList list = qvariant_cast<QStringList>(v);
    return list.join(", ");
  }

  // Fallback will always be to return an empty string (this also applies to
  // cases where the QVariant actually was an empty string!).
  return def;
}

/**
 * Constructor for the CLIRunner class.
 * Initializes the CLI runner for Ripes.
 * It configures the environment based on the provided options and prepares the
 * SystemIO streams for input and output redirection.
 *
 * @param options A struct containing the CLI options for Ripes.
 */
CLIRunner::CLIRunner(const CLIModeOptions &options)
    : QObject(), m_options(options) {
  info("Ripes CLI mode", false, true);
  ProcessorHandler::selectProcessor(m_options.proc, m_options.isaExtensions,
                                    m_options.regInit);

  // Connect systemIO output to stdout.
  connect(&SystemIO::get(), &SystemIO::doPrint, this, [&](auto text) {
    std::cout << text.toStdString();
    std::flush(std::cout);
  });

  // Handle systemIO input in stdin
  SystemIO::setCLIInput();
}

/**
 * Main execution method for the CLI runner.
 * Runs the CLI process in three phases: process input, run model, and post-run.
 * Checks after each phase that the execution was successful, and returns 1 if
 * an error occurs during any phase.
 *
 * @return 0 on success, or 1 if an error occurs during any phase.
 */
int CLIRunner::run() {
  if (processInput())
    return 1;

  if (runModel())
    return 1;

  if (postRun())
    return 1;

  return 0;
}

/**
 * Processes the input file based on the file source type in the provided CLI
 * options. The method prepares the program for the execution by assembling,
 * compiling, or loading the input file (based on the source type).
 *
 * @return 0 on success, or 1 if an error occurs during input file processing.
 */
int CLIRunner::processInput() {
  info("Processing input file", false, true);

  switch (m_options.srcType) {
  case SourceType::Assembly: {
    info("Assembling input file '" + m_options.src + "'");
    QFile inputFile(m_options.src);
    if (!inputFile.open(QIODevice::ReadOnly)) {
      error("Failed to open input file");
      return 1;
    }
    auto res = ProcessorHandler::getAssembler()->assembleRaw(
        inputFile.readAll(), &IOManager::get().assemblerSymbols());
    if (res.errors.size() == 0)
      ProcessorHandler::loadProgram(std::make_shared<Program>(res.program));
    else {
      error("Error during assembly:");
      for (auto &err : res.errors)
        info(err.errorMessage(), true);
      return 1;
    }
    break;
  };
  case SourceType::FlatBinary: {
    info("Loading binary file '" + m_options.src + "'");
    Program p;
    QString err = loadFlatBinaryFile(p, m_options.src, 0, 0);
    if (!err.isEmpty()) {
      error(err);
      return 1;
    }
    ProcessorHandler::loadProgram(std::make_shared<Program>(p));
    break;
  }
  case SourceType::InternalELF:
  case SourceType::ExternalELF: {
    // Combining cases for InternalELF and ExternalELF because they share the
    // same actions
    info("Loading elf file '" + m_options.src + "'");
    Program p;
    QFile file(m_options.src);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      error("Could not open file " + file.fileName());
      return 1;
    }
    auto info = LoadDialog::validateELFFile(
        file); // Checking the validity of the ELF file.
    if (!info.valid) {
      error(info.errorMessage);
      return 1;
    }
    if (!loadElfFile(p, file)) {
      error("Error while loading ELF file: '" + m_options.src + "'");
      return 1;
    }
    ProcessorHandler::loadProgram(std::make_shared<Program>(p));
    break;
  }
  case SourceType::C: {
    // For SourceType::C:
    // - Compile the C file, disabling graphical components.
    // - Treat the compiled file as an ExternalELF and process it.
    // - Display error output if compilation fails.
    info("Loading C file '" + m_options.src + "'");
    if (!CCManager::get().hasValidCC()) {
      error("No C compiler set.");
      return 1;
    }
    QFile file(m_options.src);
    if (!file.open(QIODevice::ReadOnly)) {
      error("Could not open file " + file.fileName());
      return 1;
    }
    QString fileContent = file.readAll();
    file.close();
    auto res = CCManager::get().compileRaw(fileContent, QString(),
                                           /* enableGUI = */ false);
    if (res.success) {
      m_options.src = res.outFile;
      m_options.srcType = SourceType::ExternalELF;
      processInput();
    } else if (!res.aborted) {
      error("Compilation failed. Error output was: " + CCManager::getError());
    }
    res.clean();
    break;
  }
  default:
    assert(false &&
           "Command-line support for this source type is not yet implemented");
  }

  return 0;
}

/**
 * Runs the processor model for the loaded program until the program is
 * finished (so ProcessorHandler::runFinished signal is emitted)
 *
 * @return 0 on success, or 1 if an error occurs during model execution.
 */
int CLIRunner::runModel() {
  info("Running model", false, true);

  QEventLoop loop;
  QObject::connect(ProcessorHandler::get(), &ProcessorHandler::runFinished,
                   &loop, &QEventLoop::quit);
  bool hadTimeout = false;
  QTimer timeoutTimer;
  timeoutTimer.setSingleShot(true);
  QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
  QObject::connect(&timeoutTimer, &QTimer::timeout, &loop,
                   [&]() { hadTimeout = true; });

  // Display status info every second when verbose output is enabled.
  QTimer infoTimer;
  QElapsedTimer elapsed;
  elapsed.start();
  infoTimer.connect(&infoTimer, &QTimer::timeout, this, [&]() {
    QTime timeFormat(0, 0);
    QString infostr =
        timeFormat.addMSecs(elapsed.elapsed()).toString("hh:mm:ss");
    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    const auto instrsRetired =
        ProcessorHandler::getProcessor()->getInstructionsRetired();
    infostr += "\tcycles: " + QString::number(cycleCount);
    infostr += "\tretired: " + QString::number(instrsRetired);
    info(infostr, false, false);
  });

  if (m_options.verbose)
    infoTimer.start(1000);

  // Start simulation
  ProcessorHandler::run();
  if (m_options.timeout != 0)
    timeoutTimer.start(m_options.timeout);
  loop.exec();

  // Event loop finished either by processor finishing or timeout. Determine the
  // cause and act.

  timeoutTimer.stop();
  infoTimer.stop();
  if (hadTimeout) {
    ProcessorHandler::stopRun();
    error("Simulation did not finish within the specified timeout (" +
          QString::number(m_options.timeout) + " ms)");
    return 1;
  }

  return 0;
}

/**
 * Handles post-execution tasks.
 * Open output file (if specified) or defaults to stdout and prints telemetry
 * data in either JSON or unstructured format.
 *
 * @return 0 on success, or 1 if an error occurs during post run tasks.
 */
int CLIRunner::postRun() {
  info("Post-run", false, true);

  // Open output stream
  std::unique_ptr<QTextStream> stream;
  std::unique_ptr<QFile> outputFile;
  if (m_options.outputFile.isEmpty()) {
    stream = std::make_unique<QTextStream>(stdout, QIODevice::WriteOnly);
  } else {
    outputFile = std::make_unique<QFile>(m_options.outputFile);
    if (!outputFile->open(QIODevice::Truncate | QIODevice::Text |
                          QIODevice::WriteOnly)) {
      error("Failed to open output file");
      return 1;
    }
    stream = std::make_unique<QTextStream>(outputFile.get());
  }

  if (m_options.jsonOutput) {
    // Telemetry output
    QJsonObject jsonOutput;
    for (auto &telemetry : m_options.telemetry)
      if (telemetry->isEnabled())
        jsonOutput.insert(
            telemetry->prettyKey(),
            QJsonValue::fromVariant(telemetry->report(/*json=*/true)));
    *stream << QJsonDocument(jsonOutput).toJson(QJsonDocument::Indented);
  } else {
    // Telemetry output
    for (auto &telemetry : m_options.telemetry)
      if (telemetry->isEnabled()) {
        *stream << "===== " << telemetry->description() << "\n";
        QVariant reportedValue = telemetry->report(/*json=*/false);
        *stream << qVariantToString(reportedValue) << "\n";
      }
  }

  // Close output file if necessary
  if (!m_options.outputFile.isEmpty())
    outputFile->close();

  return 0;
}

/**
 * Outputs an informational message to the standard output.
 * For formatting purposes the message can include a header or a specified
 * prefix.
 *
 * @param msg The QString message to print.
 * @param alwaysPrint If true, the message is printed regardless if the verbose
 * option is disabled.
 * @param header If true, the message is surrounded by a decorative header.
 * @param prefix The prefix for the message, added only if "header" is false.
 */
void CLIRunner::info(QString msg, bool alwaysPrint, bool header,
                     const QString &prefix) {

  if (m_options.verbose || alwaysPrint) {
    if (header) {
      int headerWidth = 80;
      int headerLength = msg.length();
      int headerSpaces = (headerWidth - headerLength) / 2 - 2;
      msg.prepend(" ");
      msg.append(" ");
      for (int i = 0; i < headerSpaces; i++) {
        msg.prepend("=");
        msg.append("=");
      }
    } else
      msg.prepend(prefix + ": ");
    std::cout << msg.toStdString() << std::endl;
  }
}

/**
 * Prints an error message to stdout with an "ERROR" prefix.
 *
 * @param msg The error message to print.
 */
void CLIRunner::error(const QString &msg) { info(msg, true, false, "ERROR"); }

} // namespace Ripes
