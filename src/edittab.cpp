#include "edittab.h"
#include "ui_edittab.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include "assembler/program.h"

#include "ccmanager.h"
#include "cli/programutilities.h"
#include "compilererrordialog.h"
#include "editor/codeeditor.h"
#include "io/iomanager.h"
#include "processorhandler.h"
#include "ripessettings.h"
#include "symbolnavigator.h"
#include "wasmSupport.h"

namespace Ripes {

EditTab::EditTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), m_ui(new Ui::EditTab) {
  m_ui->setupUi(this);

  m_sourceErrors = std::make_shared<Errors>();
  m_ui->codeEditor->setErrors(m_sourceErrors);

  m_symbolNavigatorAction = new QAction(this);
  m_symbolNavigatorAction->setIcon(QIcon(":/icons/compass.svg"));
  m_symbolNavigatorAction->setText("Show symbol navigator");
  connect(m_symbolNavigatorAction, &QAction::triggered, this,
          &EditTab::showSymbolNavigator);
  m_ui->navigatorButton->setDefaultAction(m_symbolNavigatorAction);

  m_followAction = new QAction(this);
  m_followAction->setIcon(QIcon(":/icons/trace.svg"));
  m_followAction->setCheckable(true);
  m_followAction->setToolTip(
      "Follow program execution.\nEnsures that the instruction present in "
      "the\nfirst stage of the processor is "
      "always visible\nin the disassembled view.");
  m_ui->followButton->setDefaultAction(m_followAction);
  connect(m_followAction, &QAction::triggered, m_ui->programViewer,
          &ProgramViewer::setFollowEnabled);
  m_followAction->setChecked(
      RipesSettings::value(RIPES_SETTING_FOLLOW_EXEC).toBool());

  m_buildAction = new QAction(this);
  m_buildAction->setIcon(QIcon(":/icons/build.svg"));
  m_buildAction->setEnabled(false);
  m_buildAction->setShortcut(QKeySequence("Ctrl+B"));
  m_buildAction->setText("Compile C program (" +
                         m_buildAction->shortcut().toString() + ")");
  connect(m_buildAction, &QAction::triggered, this, &EditTab::compile);
  m_toolbar->addAction(m_buildAction);

  m_ui->codeEditor->document()->setPlainText(
      RipesSettings::value(RIPES_SETTING_SOURCECODE).toString());

  connect(
      RipesSettings::getObserver(RIPES_SETTING_EDITORREGS),
      &SettingObserver::modified, m_ui->registers,
      [=](const QVariant &val) { m_ui->registers->setVisible(val.toBool()); });
  RipesSettings::getObserver(RIPES_SETTING_EDITORREGS)->trigger();

  connect(RipesSettings::getObserver(RIPES_SETTING_EDITORCONSOLE),
          &SettingObserver::modified, m_ui->console, [=](const QVariant &val) {
            m_ui->console->setVisible(val.toBool());
          });
  RipesSettings::getObserver(RIPES_SETTING_EDITORCONSOLE)->trigger();

  connect(m_ui->enableEditor, &QPushButton::clicked, this,
          &EditTab::enableAssemblyInput);
  connect(m_ui->codeEditor, &CodeEditor::timedTextChanged, this,
          &EditTab::sourceCodeChanged);

  m_ui->programViewer->setReadOnly(true);

  connect(m_ui->setAssemblyInput, &QRadioButton::toggled, this,
          &EditTab::sourceTypeChanged);
  connect(m_ui->setCInput, &QRadioButton::toggled, this,
          &EditTab::sourceTypeChanged);
  connect(m_ui->setCInput, &QRadioButton::toggled, m_buildAction,
          &QAction::setEnabled);
  // C compiler is not yet supported on WASM.
  disableIfWasm(m_ui->setCInput);

  // Ensure that changes to the current compiler path will disable C input, if
  // the compiler is invalid
  connect(&CCManager::get(), &CCManager::ccChanged, this, [=](auto res) {
    if (!res.success) {
      m_ui->setAssemblyInput->setChecked(true);
    }
  });

  // During processor running, it should not be possible to build the program
  connect(ProcessorHandler::get(), &ProcessorHandler::runStarted, m_buildAction,
          [=] { m_buildAction->setEnabled(false); });
  connect(ProcessorHandler::get(), &ProcessorHandler::runFinished,
          m_buildAction,
          [=] { m_buildAction->setEnabled(m_ui->setCInput->isChecked()); });
  connect(ProcessorHandler::get(), &ProcessorHandler::procStateChangedNonRun,
          this, &EditTab::updateProgramViewerHighlighting);
  connect(ProcessorHandler::get(), &ProcessorHandler::programChanged, this,
          &EditTab::updateProgramViewer);
  connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this,
          &EditTab::onProcessorChanged);
  onProcessorChanged();
  sourceTypeChanged();
  enableEditor();

  // Make left-hand side widgets stretch wrt. registers
  m_ui->toplevelSplitter->setStretchFactor(0, 2);
  m_ui->toplevelSplitter->setStretchFactor(1, 0);

  // make editor and program viewer stretch wrt. console
  m_ui->leftSpliiter->setStretchFactor(0, INT_MAX);
  m_ui->leftSpliiter->setStretchFactor(1, 0);

  // make editor and program viewer stretch equally wrt. each other
  m_ui->editorSplitter->setStretchFactor(0, 2);
  m_ui->editorSplitter->setStretchFactor(1, 2);

  switch (RipesSettings::value(RIPES_SETTING_INPUT_TYPE).toInt()) {
  case SourceType::Assembly: {
    m_ui->setAssemblyInput->toggle();
    break;
  }
  case SourceType::C: {
    m_ui->setCInput->toggle();
    break;
  }
  default:
    break;
  }
}

void EditTab::showSymbolNavigator() {
  if (auto program = ProcessorHandler::getProgram()) {
    SymbolNavigator nav(program->symbols, this);
    if (nav.exec()) {
      m_ui->programViewer->setCenterAddress(nav.getSelectedSymbolAddress());
    }
  }
}

void EditTab::onSave() { m_ui->codeEditor->onSave(); }

bool EditTab::loadExternalFile(const LoadFileParams &params) {
  if (params.type == SourceType::C) {
    // Try to enable C input and verify that source type was changed
    // successfully. This allows us to trigger the message box associated with C
    // input, if no compiler is set. If so, load the file.
    enableEditor();
    m_ui->setCInput->setChecked(true);
    if (m_currentSourceType == SourceType::C) {
      return loadFile(params);
    }
  } else if (params.type == SourceType::Assembly) {
    m_ui->setAssemblyInput->setChecked(true);
    return loadFile(params);
  } else {
    m_currentSourceType = params.type;
    return loadFile(params);
  }
  return false;
}

bool EditTab::loadFile(const LoadFileParams &fileParams) {
  QFile file(fileParams.filepath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Error",
                         "Error: Could not open file " + fileParams.filepath);
    return false;
  }

  bool success = true;
  auto loadedProgram = std::make_shared<Program>();
  switch (fileParams.type) {
  case SourceType::C:
  case SourceType::Assembly: {
    success &= loadSourceFile(*loadedProgram, file);
    break;
  }
  case SourceType::FlatBinary: {
    QString err = loadFlatBinaryFile(*loadedProgram, file.fileName(),
                                     fileParams.binaryEntryPoint,
                                     fileParams.binaryLoadAt);
    success = err.isEmpty();
    if (!success) {
      QMessageBox::warning(this, "Error", err);
    } else {
      m_ui->curInputSrcLabel->setText("Flat binary");
      m_ui->inputSrcPath->setText(file.fileName());
      disableEditor();
    }
    break;
  }
  case SourceType::ExternalELF: {
    // Since there is no related source code for an externally compiled ELF, the
    // editor is disabled
    disableEditor();
    // Intentional fallthrough to InternalELF case for shared actions.
  }
  case SourceType::InternalELF: {
    success &= loadElfFile(*loadedProgram, file);
    m_ui->curInputSrcLabel->setText("Executable (ELF)");
    m_ui->inputSrcPath->setText(file.fileName());
    break;
  }
  }

  if (success) {
    // Move the shared pointer to be the current active program
    ProcessorHandler::loadProgram(loadedProgram);
  } else {
    QMessageBox::warning(this, "Error",
                         "Error: Could not load file " + fileParams.filepath);
  }
  file.close();
  return success;
}

QString EditTab::getAssemblyText() { return m_ui->codeEditor->toPlainText(); }

void EditTab::clearAssemblyEditor() { m_ui->codeEditor->clear(); }

void EditTab::updateProgramViewerHighlighting() {
  if (isVisible()) {
    m_ui->programViewer->updateHighlightedAddresses();
  }
}

Errors *EditTab::errors() {
  if (m_sourceErrors->empty())
    return nullptr;
  return m_sourceErrors.get();
}

void EditTab::sourceTypeChanged() {
  if (!m_editorEnabled) {
    // Do nothing; editor is currently disabled so we should not care about
    // updating our source type being the code editor. sourceTypeChanged() will
    // be re-executed once the editor is reenabled.
    return;
  }

  // Clear any errors (ie. when assembler had errors, and now switching to C)
  *m_sourceErrors = {};

  // Validate source type selection
  if (m_ui->setAssemblyInput->isChecked()) {
    m_currentSourceType = SourceType::Assembly;
  } else if (m_ui->setCInput->isChecked()) {
    // Ensure that we have a validated C compiler available
    if (!CCManager::get().hasValidCC()) {
      QMessageBox::warning(this, "Error",
                           "No C compiler set.\n\nProvide a path to a valid C "
                           "compiler under:\n Edit->Settings->Editor");
      // Re-enable assembly input
      m_ui->setAssemblyInput->setChecked(true);
      return;
    } else {
      m_currentSourceType = SourceType::C;
    }
  }

  // Notify the source type change to the code editor
  m_ui->codeEditor->setSourceType(
      m_currentSourceType, ProcessorHandler::getAssembler()->getOpcodes());

  // And store in settings
  RipesSettings::setValue(RIPES_SETTING_INPUT_TYPE, m_currentSourceType);
}

void EditTab::onProcessorChanged() {
  // Notify a possible assembler change to the code editor - opcodes might have
  // been added or removed which must be reflected in the syntax highlighter
  m_ui->codeEditor->setSourceType(
      m_currentSourceType, ProcessorHandler::getAssembler()->getOpcodes());

  // Try reassembling
  sourceCodeChanged();
}

void EditTab::sourceCodeChanged() {
  auto source = m_ui->codeEditor->document()->toPlainText();
  // Update the editor text setting for program persistance.
  RipesSettings::setValue(RIPES_SETTING_SOURCECODE, source);
  switch (m_currentSourceType) {
  case SourceType::Assembly:
    assemble(source);
    break;
  default:
    // Do nothing, either some external program is loaded or, if compiling from
    // C, the user shall manually select to build
    break;
  }
}

void EditTab::assemble(const QString &source) {
  auto res = ProcessorHandler::getAssembler()->assembleRaw(
      source, &IOManager::get().assemblerSymbols());
  *m_sourceErrors = res.errors;
  if (m_sourceErrors->size() == 0) {
    ProcessorHandler::loadProgram(std::make_shared<Program>(res.program));
  } else {
    // Errors occured; rehighlight will reflect current m_sourceErrors in the
    // editor.
#ifndef NDEBUG
    // Ensure only valid error messages are present.
    for (auto &err : *m_sourceErrors)
      assert(err.isKnownSourceLine());
#endif
  }
  m_ui->codeEditor->rehighlight();
}

void EditTab::compile() {
  // We don't care about asking our editor for syntax accepted, since there is
  // no C-syntax checking in Ripes.
  auto res = CCManager::get().compile(m_ui->codeEditor->document());
  if (res.success) {
    // Compilation successful; load file through standard file loading functions
    LoadFileParams params;
    params.filepath = res.outFile;
    params.type = SourceType::InternalELF;
    loadFile(params);
    // Clean up temporary source and output files
  } else if (!res.aborted) {
    CompilerErrorDialog errDiag(this);
    errDiag.setText("Compilation failed. Error output was:");
    errDiag.setErrorText(CCManager::getError());
    errDiag.exec();
  }
  res.clean();
}

EditTab::~EditTab() { delete m_ui; }

void EditTab::newProgram() {
  m_ui->codeEditor->clear();
  enableAssemblyInput();
}

void EditTab::setSourceText(const QString &text) {
  m_ui->codeEditor->clear();
  m_ui->codeEditor->setPlainText(text);
}

void EditTab::enableAssemblyInput() {
  // Clear currently loaded binary/ELF program
  updateProgramViewer();
  enableEditor();
}

void EditTab::updateProgramViewer() {
  m_ui->programViewer->updateProgram(
      !m_ui->disassembledViewButton->isChecked());
}

void EditTab::enableEditor() {
  m_editorEnabled = true;
  m_ui->editorStackedWidget->setCurrentIndex(0);
  sourceTypeChanged();
  emit editorStateChanged(m_editorEnabled);
}

void EditTab::disableEditor() {
  m_ui->editorStackedWidget->setCurrentIndex(1);
  clearAssemblyEditor();
  m_editorEnabled = false;
  emit editorStateChanged(m_editorEnabled);
}

void EditTab::on_disassembledViewButton_toggled() { updateProgramViewer(); }

void EditTab::loadSourceText(const QString &text) {
  enableEditor();
  setSourceText(text);
}

bool EditTab::loadSourceFile(Program &, QFile &file) {
  loadSourceText(file.readAll());
  return true;
}

} // namespace Ripes
