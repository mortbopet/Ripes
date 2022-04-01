#pragma once

#include <QTextStream>

#include "processorhandler.h"

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

  void enable() { m_enabled = true; }
  void disable() { m_enabled = false; }
  bool isEnabled() const { return m_enabled; }

private:
  bool m_enabled = false;
};

class CPITelemetry : public Telemetry {
  QString key() const override { return "cpi"; }
  QString description() const override {
    return "Report cycles per instruction";
  }
  void report(QTextStream &out) override {
    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    const auto instrsRetired =
        ProcessorHandler::getProcessor()->getInstructionsRetired();
    const double cpi =
        static_cast<double>(cycleCount) / static_cast<double>(instrsRetired);
    out << "CPI: " << QString::number(cpi) << "\n";
  }
};

class IPCTelemetry : public Telemetry {
  QString key() const override { return "ipc"; }
  QString description() const override {
    return "Report instructions per cycle";
  }
  void report(QTextStream &out) override {
    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    const auto instrsRetired =
        ProcessorHandler::getProcessor()->getInstructionsRetired();
    const double cpi =
        static_cast<double>(cycleCount) / static_cast<double>(instrsRetired);
    const double ipc = 1 / cpi;
    out << "IPC: " << QString::number(ipc) << "\n";
  }
};

class CyclesTelemetry : public Telemetry {
  QString key() const override { return "cycles"; }
  QString description() const override {
    return "Report total number of cycles";
  }
  void report(QTextStream &out) override {
    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    out << "Cycles: " << QString::number(cycleCount) << "\n";
  }
};

} // namespace Ripes
