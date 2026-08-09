#pragma once

#include <QTextStream>

#include "pipelinediagrammodel.h"
#include "processorhandler.h"
#include "radix.h"

#include <memory>
#include <utility>
#include <vector>

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
    auto isa = ProcessorHandler::currentISA();

    if (json) {
      for (const auto &regFile : isa->regInfos()) {
        for (unsigned i = 0; i < regFile->regCnt(); i++) {
          registerMap[regFile->regName(i)] = QVariant::fromValue(
              ProcessorHandler::getRegisterValue(regFile->regFileName(), i));
        }
      }
      return registerMap;
    } else {
      QString outStr;
      QTextStream out(&outStr);
      for (const auto &regFile : isa->regInfos()) {
        for (unsigned i = 0; i < regFile->regCnt(); i++) {
          auto v =
              ProcessorHandler::getRegisterValue(regFile->regFileName(), i);
          out << regFile->regName(i) << ":\t"
              << encodeRadixValue(v, Radix::Signed, isa->bytes()) << "\t";
          out << "(" << encodeRadixValue(v, Radix::Hex, isa->bytes()) << ")\n";
        }
      }
      return outStr;
    }
  }
};

/// Reports the contents of memory at requested symbols/labels. Values are
/// read non-perturbingly (readMemConst), so a dump cannot itself change
/// simulator state or cache statistics.
class MemoryTelemetry : public Telemetry {
public:
  // spec: "<label>[:<nwords>]" -- nwords defaults to 1 if omitted.
  void addDump(const QString &spec) {
    QString name = spec;
    unsigned count = 1;
    int colon = spec.indexOf(':');
    if (colon >= 0) {
      name = spec.left(colon);
      bool ok = false;
      unsigned parsed = spec.mid(colon + 1).toUInt(&ok);
      if (ok && parsed > 0)
        count = parsed;
    }
    m_specs.push_back({name, count});
  }

  QString key() const override { return "mem"; }
  QString description() const override {
    return "memory contents at requested labels";
  }

  QVariant report(bool /*json*/) override {
    QMap<QString, AInt> byName;
    if (auto program = ProcessorHandler::getProgram()) {
      for (const auto &[addr, sym] : program->symbols)
        byName[sym.v] = addr;
    }

    QVariantMap out;
    for (const auto &specPair : m_specs) {
      const QString &name = specPair.first;
      const unsigned count = specPair.second;
      if (!byName.contains(name)) {
        // Absent symbol => ABI violation; report explicitly rather than
        // silently omitting the key.
        out[name] = QVariant();
        continue;
      }
      const AInt base = byName[name];
      QVariantList words;
      for (unsigned i = 0; i < count; ++i)
        words.append(QVariant::fromValue(
            ProcessorHandler::getMemory().readMemConst(base + i * 4, 4)));
      out[name] = words;
    }
    return out;
  }

private:
  std::vector<std::pair<QString, unsigned>> m_specs;
};

/// Reports why/how the simulation stopped. Distinguishes a deterministic
/// --max-cycles bound from normal completion, so grading does not depend on
/// wall-clock timing (see --timeout, which is host-load dependent).
class ExitTelemetry : public Telemetry {
public:
  QString key() const override { return "exit"; }
  QString description() const override {
    return "exit reason (normal, max_cycles)";
  }
  QVariant report(bool /*json*/) override {
    QVariantMap m;
    m["reason"] =
        ProcessorHandler::maxCyclesExceeded() ? "max_cycles" : "normal";
    m["cycles"] =
        QVariant::fromValue(ProcessorHandler::getProcessor()->getCycleCount());
    return m;
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
