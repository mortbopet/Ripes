#pragma once

#include <QTextStream>

#include "pipelinediagrammodel.h"
#include "processorhandler.h"
#include "radix.h"

#include <memory>

namespace Ripes {

/// A class for
class Telemetry {
public:
  Telemetry() {}
  virtual ~Telemetry(){};

  // Report this telemetry to the provided output stream.
  virtual void report(QTextStream &out) = 0;

  // Returns the name of this telemetry.
  virtual QString key() const = 0;

  // Returns the description of this telemetry.
  virtual QString description() const = 0;

  virtual void enable() { m_enabled = true; }
  virtual void disable() { m_enabled = false; }
  bool isEnabled() const { return m_enabled; }

private:
  bool m_enabled = false;
};

class CPITelemetry : public Telemetry {
  QString key() const override { return "cpi"; }
  QString description() const override {
    return "cycles per instruction (CPI)";
  }
  void report(QTextStream &out) override {
    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    const auto instrsRetired =
        ProcessorHandler::getProcessor()->getInstructionsRetired();
    const double cpi =
        static_cast<double>(cycleCount) / static_cast<double>(instrsRetired);
    out << QString::number(cpi) << "\n";
  }
};

class IPCTelemetry : public Telemetry {
  QString key() const override { return "ipc"; }
  QString description() const override {
    return "instructions per cycle (IPC)";
  }
  void report(QTextStream &out) override {
    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    const auto instrsRetired =
        ProcessorHandler::getProcessor()->getInstructionsRetired();
    const double cpi =
        static_cast<double>(cycleCount) / static_cast<double>(instrsRetired);
    const double ipc = 1 / cpi;
    out << QString::number(ipc) << "\n";
  }
};

class CyclesTelemetry : public Telemetry {
  QString key() const override { return "cycles"; }
  QString description() const override { return "cycles"; }
  void report(QTextStream &out) override {
    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    out << QString::number(cycleCount) << "\n";
  }
};

class InstrsRetiredTelemetry : public Telemetry {
  QString key() const override { return "iret"; }
  QString description() const override { return "instructions retired"; }
  void report(QTextStream &out) override {
    const auto instrsRetired =
        ProcessorHandler::getProcessor()->getInstructionsRetired();
    out << QString::number(instrsRetired) << "\n";
  }
};

class PipelineTelemetry : public Telemetry {
public:
  PipelineTelemetry() {}
  void enable() override {
    // The PipelineDiagramModel will automatically, upon construction, connect
    // to the ProcessorHandler and record information during execution.
    m_pipelineDiagramModel = std::make_shared<PipelineDiagramModel>();
    Telemetry::enable();
  }

  QString key() const override { return "pipeline"; }
  QString description() const override { return "pipeline state"; }
  void report(QTextStream &out) override {
    // Simply grab the current state of the pipeline diagram model and print it.
    out << m_pipelineDiagramModel->toString() << "\n";
  }

private:
  std::shared_ptr<PipelineDiagramModel> m_pipelineDiagramModel;
};

class RegisterTelemetry : public Telemetry {
public:
  RegisterTelemetry() {}
  QString key() const override { return "regs"; }
  QString description() const override { return "register values"; }
  void report(QTextStream &out) override {
    auto *isa = ProcessorHandler::currentISA();
    for (unsigned i = 0; i < isa->regCnt(); i++) {
      auto v = ProcessorHandler::getRegisterValue(RegisterFileType::GPR, i);
      out << isa->regName(i) << ":\t"
          << encodeRadixValue(v, Radix::Signed, isa->bytes()) << "\t";
      out << "(" << encodeRadixValue(v, Radix::Hex, isa->bytes()) << ")\n";
    }
  }
};

} // namespace Ripes
