#pragma once

#include <QAction>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QWidget>

#include <chrono>

#include "isa/isa_types.h"
#include "processors/interface/ripesprocessor.h"
#include "ripestab.h"

namespace vsrtl {
class VSRTLWidget;
class Label;
} // namespace vsrtl

namespace Ripes {

namespace Ui {
class ProcessorTab;
}

class InstructionModel;
class RegisterModel;
class PipelineDiagramModel;
struct Layout;

class ProcessorTab : public RipesTab {
  friend class RunDialog;
  friend class MainWindow;
  Q_OBJECT

public:
  ProcessorTab(QToolBar *controlToolbar, QToolBar *additionalToolbar,
               QWidget *parent = nullptr);
  ~ProcessorTab() override;

  void initRegWidget();

public slots:
  void pause();
  void restart();
  void reset();
  void reverse();
  void processorFinished();
  void runFinished();
  void updateStatistics();
  void updateInstructionLabels();
  void fitToScreen();

  /// Sets the processor diagram dark mode. Used to keep the diagram in sync
  /// with the application-wide color scheme.
  void setDarkmode(bool enabled);

  void processorSelection();

private slots:
  void run(bool state);
  void autoClock(bool state);
  void autoClockTimeout();
  void setInstructionViewCenterRow(int row);
  void showPipelineDiagram();

private:
  void setupSimulatorActions(QToolBar *controlToolbar);
  /// Regenerates the simulator-control toolbar icons in the current theme's
  /// foreground color. Called on construction and whenever the theme changes.
  void updateToolbarIcons();
  void enableSimulatorControls();
  void updateInstructionModel();
  void updateRegisterModel();
  void loadLayout(const Layout &);
  void loadProcessorToWidget(const Layout *);

  /// Resets the state used to compute the smoothed clock-rate estimate, so that
  /// a subsequent run starts averaging afresh.
  void resetClockRateEstimate();

  Ui::ProcessorTab *m_ui = nullptr;
  InstructionModel *m_instrModel = nullptr;
  PipelineDiagramModel *m_stageModel = nullptr;

  vsrtl::VSRTLWidget *m_vsrtlWidget = nullptr;

  std::map<StageIndex, vsrtl::Label *> m_stageInstructionLabels;

  QTimer *m_statUpdateTimer;

  // State for computing a stable, smoothed clock rate during a run. Showing the
  // instantaneous cycles/second between successive stat-timer ticks is very
  // noisy; instead the displayed rate is averaged over a fixed time window and
  // lightly smoothed across windows (see updateStatistics()).
  std::chrono::steady_clock::time_point m_clockRateWindowStart;
  long long m_clockRateWindowStartCycle = 0;
  double m_smoothedClockRate = 0.0;
  bool m_clockRateWindowValid = false;
  bool m_haveSmoothedClockRate = false;

  // Actions
  QAction *m_selectProcessorAction = nullptr;
  QAction *m_clockAction = nullptr;
  QAction *m_autoClockAction = nullptr;
  QAction *m_runAction = nullptr;
  QAction *m_displayValuesAction = nullptr;
  QAction *m_pipelineDiagramAction = nullptr;
  QAction *m_reverseAction = nullptr;
  QAction *m_resetAction = nullptr;
  QTimer *m_autoClockTimer = nullptr;

  QSpinBox *m_autoClockInterval = nullptr;
};
} // namespace Ripes
