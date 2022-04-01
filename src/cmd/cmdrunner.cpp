#include "cmdrunner.h"
#include "io/iomanager.h"
#include "processorhandler.h"
#include "programutilities.h"
#include "syscall/systemio.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace Ripes {

// An extended QVariant-to-string convertion method which handles a few special
// cases.
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

CmdRunner::CmdRunner(const CmdModeOptions &options)
    : QObject(), m_options(options) {
  info("Ripes command-line mode", false, true);
  ProcessorHandler::selectProcessor(m_options.proc, m_options.isaExtensions);

  // Connect systemIO output to stdout.
  connect(&SystemIO::get(), &SystemIO::doPrint, this, [&](auto text) {
    std::cout << text.toStdString();
    std::flush(std::cout);
  });

  // TODO: how to handle system input?
}

int CmdRunner::run() {
  if (processInput())
    return 1;

  if (runModel())
    return 1;

  if (postRun())
    return 1;

  return 0;
}

int CmdRunner::processInput() {
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
  default:
    assert(false &&
           "Command-line support for this source type is not yet implemented");
  }

  return 0;
}

int CmdRunner::runModel() {
  info("Running model", false, true);
  ProcessorHandler::run();

  // Wait until receiving ProcessorHandler::runFinished signal
  // before proceeding.
  QEventLoop loop;
  QObject::connect(ProcessorHandler::get(), &ProcessorHandler::runFinished,
                   &loop, &QEventLoop::quit);
  loop.exec();
  return 0;
}

int CmdRunner::postRun() {
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
        jsonOutput.insert(telemetry->prettyKey(),
                          QJsonValue::fromVariant(telemetry->report()));
    *stream << QJsonDocument(jsonOutput).toJson(QJsonDocument::Indented);
  } else {
    // Telemetry output
    for (auto &telemetry : m_options.telemetry)
      if (telemetry->isEnabled()) {
        *stream << "===== " << telemetry->description() << "\n";
        QVariant reportedValue = telemetry->report();
        *stream << qVariantToString(reportedValue) << "\n";
      }
  }

  // Close output file if necessary
  if (!m_options.outputFile.isEmpty())
    outputFile->close();

  return 0;
}

void CmdRunner::info(QString msg, bool alwaysPrint, bool header) {

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
      msg.prepend("INFO: ");
    std::cout << msg.toStdString() << std::endl;
  }
}

void CmdRunner::error(const QString &msg) {
  info("ERROR: " + msg, true, false);
}

} // namespace Ripes
