#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QMessageBox>
#include <QResource>
#include <QTimer>
#include <iostream>

#include "src/cmd/cmdoptions.h"
#include "src/cmd/cmdrunner.h"
#include "src/mainwindow.h"

using namespace std;

void initParser(QCommandLineParser &parser, Ripes::CmdModeOptions &options) {
  QString helpText =
      "The command line interface allows for assembling/compiling/executing a "
      "\n"
      "program on an arbitrary processor model and subsequent reporting of \n"
      "execution telemetry.\nCommand line mode is enabled when the '--mode "
      "sh' argument is provided.";

  helpText.prepend("Ripes command line interface.\n");
  parser.setApplicationDescription(helpText);
  QCommandLineOption modeOption("mode", "Ripes mode [gui, sh]", "mode", "gui");
  parser.addOption(modeOption);
  Ripes::addCmdOptions(parser, options);
}

enum CommandLineParseResult {
  CommandLineError,
  CommandLineHelpRequested,
  CommandLineGUI,
  CommandLineCMD
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser,
                                        QString &errorMessage) {
  const QCommandLineOption helpOption = parser.addHelpOption();
  if (!parser.parse(QCoreApplication::arguments())) {
    errorMessage = parser.errorText();
    return CommandLineError;
  }

  if (parser.isSet(helpOption))
    return CommandLineHelpRequested;

  if (parser.value("mode") == "gui")
    return CommandLineGUI;
  else if (parser.value("mode") == "sh")
    return CommandLineCMD;
  else {
    errorMessage = "Invalid mode: " + parser.value("mode");
    return CommandLineError;
  }
}

int guiMode(QApplication &app) {
  Ripes::MainWindow m;

  // The following sequence of events manages to successfully start the
  // application as maximized, with the processor at a reasonable size. This has
  // been found to be specially a problem on windows.
  m.resize(800, 600);
  m.showMaximized();
  m.setWindowState(Qt::WindowMaximized);
  QTimer::singleShot(100, &m, [&m] { m.fitToView(); });

  return app.exec();
}

int cmdMode(QCommandLineParser &parser, Ripes::CmdModeOptions &options) {
  QString err;
  if (!Ripes::parseCmdOptions(parser, err, options)) {
    std::cerr << "ERROR: " << err.toStdString() << std::endl;
    parser.showHelp();
    return 0;
  }
  return Ripes::CmdRunner(options).run();
}

int main(int argc, char **argv) {
  Q_INIT_RESOURCE(icons);
  Q_INIT_RESOURCE(examples);
  Q_INIT_RESOURCE(layouts);
  Q_INIT_RESOURCE(fonts);

  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("Ripes");

  QCommandLineParser parser;
  Ripes::CmdModeOptions options;
  initParser(parser, options);
  QString err;
  switch (parseCommandLine(parser, err)) {
  case CommandLineError:
    std::cerr << "ERROR: " << err.toStdString() << std::endl;
    parser.showHelp();
    return 0;
  case CommandLineHelpRequested:
    parser.showHelp();
    return 0;
  case CommandLineGUI:
    return guiMode(app);
  case CommandLineCMD:
    return cmdMode(parser, options);
  }
}
