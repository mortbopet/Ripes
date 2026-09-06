#pragma once

#include <QLabel>
#include <QTimer>
#include <QWidget>

#include "iobase.h"

namespace Ripes {

/**
 * @brief The IOClock class
 * A real-time clock peripheral. Unlike a cycle-count-derived timer, this
 * peripheral exposes the host machine's wall-clock time, allowing simulated
 * programs to be coupled with real-time semantics (e.g. paced game loops or
 * measuring real elapsed time).
 *
 * The clock value is the number of nanoseconds since the Unix epoch, sampled
 * fresh on every read, and exposed as a 64-bit value through two 32-bit
 * registers:
 *   - TIME_LO (offset 0): the low 32 bits.
 *   - TIME_HI (offset 4): the high 32 bits.
 *
 * To obtain a coherent 64-bit value on 32-bit targets without tearing (the
 * counter advances between two 32-bit reads), reading TIME_LO atomically
 * samples the full 64-bit time, returns its low word, and latches the high word
 * so that a subsequent read of TIME_HI returns the matching high 32 bits.
 * Therefore always read TIME_LO before TIME_HI. On 64-bit targets the whole
 * value may instead be read in a single 8-byte access at offset 0.
 */
class IOClock : public IOBase {
  Q_OBJECT

public:
  IOClock(QWidget *parent);
  ~IOClock() { unregister(); }

  enum Registers { TIME_LO, TIME_HI, NREGISTERS };

  virtual unsigned byteSize() const override { return 8; }
  virtual QString description() const override;
  virtual QString baseName() const override { return "RTC"; }

  virtual const std::vector<RegDesc> &registers() const override {
    return m_regDescs;
  }

  /**
   * Hardware read/write functions
   */
  virtual VInt ioRead(AInt offset, unsigned size) override;
  virtual void ioWrite(AInt offset, VInt value, unsigned size) override;

protected:
  virtual void parameterChanged(unsigned) override { /* no parameters */ }

private:
  /// Returns the current wall-clock time as nanoseconds since the Unix epoch.
  static uint64_t nowNanos();

  /// Refreshes the human-readable time display.
  void updateDisplay();

  std::vector<RegDesc> m_regDescs;

  /// High 32 bits of the most recent TIME_LO sample, returned by a subsequent
  /// read of TIME_HI (see class documentation).
  uint32_t m_latchedHigh = 0;

  QLabel *m_epochLabel = nullptr;
  QLabel *m_humanLabel = nullptr;
  QTimer *m_displayTimer = nullptr;
};

} // namespace Ripes
