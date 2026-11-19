#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QMessageBox>
#include <QProxyStyle>
#include <QResource>
#include <QStyleOptionTitleBar>
#include <iostream>

#include "src/cli/clioptions.h"
#include "src/cli/clirunner.h"
#include "src/mainwindow.h"
#include "src/ripessettings.h"

using namespace std;

void initParser(QCommandLineParser &parser, Ripes::CLIModeOptions &options) {
  QString helpText =
      "The command line interface allows for assembling/compiling/executing a "
      "\n"
      "program on an arbitrary processor model and subsequent reporting of \n"
      "execution telemetry.\nCommand line mode is enabled when the '--mode "
      "cli' argument is provided.";

  helpText.prepend("Ripes command line interface.\n");
  parser.setApplicationDescription(helpText);
  QCommandLineOption modeOption("mode", "Ripes mode [gui, cli]", "mode", "gui");
  parser.addOption(modeOption);
  Ripes::addCLIOptions(parser, options);
}

enum CommandLineParseResult {
  CommandLineError,
  CommandLineHelpRequested,
  CommandLineGUI,
  CommandLineCLI
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
  else if (parser.value("mode") == "cli")
    return CommandLineCLI;
  else {
    errorMessage = "Invalid mode: " + parser.value("mode");
    return CommandLineError;
  }
}

namespace {

/// The Fusion style, with one fix for dark mode. Fusion draws the title of an
/// active title bar (e.g. an I/O device's MDI window) on top of a copy offset
/// by 1px, coloured with QPalette::Text, as a shadow. That colour is black in
/// light mode but white in dark mode, where the shadow then shows up as a
/// second, ghosted copy of the title (#475). Keep the shadow dark, as it is in
/// light mode. QPalette::Text is not used for anything else in the title bar.
class RipesStyle : public QProxyStyle {
public:
  RipesStyle() : QProxyStyle(QStringLiteral("Fusion")) {}

  void drawComplexControl(ComplexControl control,
                          const QStyleOptionComplex *option, QPainter *painter,
                          const QWidget *widget) const override {
    if (control == CC_TitleBar) {
      const auto *titleBar =
          qstyleoption_cast<const QStyleOptionTitleBar *>(option);
      if (titleBar && (titleBar->titleBarState & State_Active)) {
        QStyleOptionTitleBar darkShadow(*titleBar);
        darkShadow.palette.setColor(QPalette::Text, Qt::black);
        QProxyStyle::drawComplexControl(control, &darkShadow, painter, widget);
        return;
      }
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
  }
};

} // namespace

int guiMode(QApplication &app) {
  // Use the Fusion style (through RipesStyle, see above), which respects the
  // system light/dark palette and looks good with both. Combined with
  // QStyleHints::setColorScheme (see applyColorScheme) this gives the
  // application proper light/dark theming.
  QApplication::setStyle(new RipesStyle);
  Ripes::applyColorScheme();

  Ripes::MainWindow m;

#ifdef Q_OS_WASM
  // In the WASM build, we'll just want a full-screen application that can't be
  // dragged or resized by the user.
  m.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  m.showMaximized();
#else
  // Restore the previous window geometry (falls back to a maximized window on
  // first launch). The initial processor fit-to-view is performed in
  // MainWindow::showEvent, once the final window geometry has been applied.
  m.restoreWindowGeometry();
  m.show();
#endif

  return app.exec();
}

int CLIMode(QCommandLineParser &parser, Ripes::CLIModeOptions &options) {
  QString err;
  if (!Ripes::parseCLIOptions(parser, err, options)) {
    std::cerr << "ERROR: " << err.toStdString() << std::endl;
    parser.showHelp();
    return 0;
  }
  return Ripes::CLIRunner(options).run();
}

int main(int argc, char **argv) {
  Q_INIT_RESOURCE(icons);
  Q_INIT_RESOURCE(examples);
  Q_INIT_RESOURCE(layouts);
  Q_INIT_RESOURCE(fonts);

  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("Ripes");

  QCommandLineParser parser;
  Ripes::CLIModeOptions options;
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
  case CommandLineCLI:
    return CLIMode(parser, options);
  }
}
