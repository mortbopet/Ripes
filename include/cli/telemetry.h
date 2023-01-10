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

  // Returns a QVariant representing representing this telemtry. If 'json' is
  // set, indicates that the output is intended for JSON export.
  virtual QVariant report(bool /*json*/) = 0;

  // Returns the name of this telemetry.
  virtual QString key() const = 0;

  // Returns a pretty version of the primary key.
  virtual QString prettyKey() const { return key(); }

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
  QString prettyKey() const override { return "CPI"; }
  QString description() const override {
    return "cycles per instruction (CPI)";
  }

  QVariant report(bool /*json*/) override {
    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    const auto instrsRetired =
        ProcessorHandler::getProcessor()->getInstructionsRetired();
    const double cpi =
        static_cast<double>(cycleCount) / static_cast<double>(instrsRetired);
    return cpi;
  }
};

class IPCTelemetry : public Telemetry {
  QString key() const override { return "ipc"; }
  QString prettyKey() const override { return "IPC"; }
  QString description() const override {
    return "instructions per cycle (IPC)";
  }
  QVariant report(bool /*json*/) override {
    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    const auto instrsRetired =
        ProcessorHandler::getProcessor()->getInstructionsRetired();
    const double cpi =
        static_cast<double>(cycleCount) / static_cast<double>(instrsRetired);
    const double ipc = 1 / cpi;
    return ipc;
  }
};

class CyclesTelemetry : public Telemetry {
  QString key() const override { return "cycles"; }
  QString description() const override { return "cycles"; }
  QVariant report(bool /*json*/) override {
    return ProcessorHandler::getProcessor()->getCycleCount();
  }
};

class InstrsRetiredTelemetry : public Telemetry {
  QString key() const override { return "iret"; }
  QString prettyKey() const override { return "# instructions retired"; }
  QString description() const override { return "instructions retired"; }
  QVariant report(bool /*json*/) override {
    return ProcessorHandler::getProcessor()->getInstructionsRetired();
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
  QVariant report(bool /*json*/) override {
    // Simply grab the current state of the pipeline diagram model and print it.
    return m_pipelineDiagramModel->toString();
  }

private:
  std::shared_ptr<PipelineDiagramModel> m_pipelineDiagramModel;
};

class RegisterTelemetry : public Telemetry {
public:
  QString key() const override { return "regs"; }
  QString prettyKey() const override { return "registers"; }
  QString description() const override { return "register values"; }
  QVariant report(bool json) override {
    QVariantMap registerMap;
    auto *isa = ProcessorHandler::currentISA();
    if (json) {
      for (unsigned i = 0; i < isa->regCnt(); i++) {
        registerMap[isa->regName(i)] = QVariant::fromValue(
            ProcessorHandler::getRegisterValue(RegisterFileType::GPR, i));
      }
      return registerMap;
    } else {
      QString outStr;
      QTextStream out(&outStr);
      for (unsigned i = 0; i < isa->regCnt(); i++) {
        auto v = ProcessorHandler::getRegisterValue(RegisterFileType::GPR, i);
        out << isa->regName(i) << ":\t"
            << encodeRadixValue(v, Radix::Signed, isa->bytes()) << "\t";
        out << "(" << encodeRadixValue(v, Radix::Hex, isa->bytes()) << ")\n";
      }
      return outStr;
    }
  }
};

class RunInfoTelemetry : public Telemetry {
public:
  RunInfoTelemetry(QCommandLineParser *parser) {
    // Store a handle to the parser so we can look up the input file name upon
    // reporting.
    m_parser = parser;
  }
  QString key() const override { return "runinfo"; }
  QString description() const override {
    return "simulation information (processor "
           "configuration, input file, ...)";
  }
  QVariant report(bool /*json*/) override {
    QVariantMap m;
    m["processor"] = enumToString<ProcessorID>(ProcessorHandler::getID());
    m["ISA extensions"] = ProcessorHandler::currentISA()->enabledExtensions();
    m["source file"] = m_parser->value("src");
    return m;
  }

private:
  QCommandLineParser *m_parser = nullptr;
};

} // namespace Ripes
