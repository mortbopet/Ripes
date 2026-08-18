#include "ioclock.h"
#include "ioregistry.h"

#include <QDateTime>
#include <QVBoxLayout>

#include <chrono>

namespace Ripes {

IOClock::IOClock(QWidget *parent) : IOBase(IOType::RTC, parent) {
  m_regDescs = {
      RegDesc{"TIME_LO", RegDesc::RW::R, 32, TIME_LO * 4, true},
      RegDesc{"TIME_HI", RegDesc::RW::R, 32, TIME_HI * 4, true},
  };

  auto *layout = new QVBoxLayout(this);
  m_epochLabel = new QLabel(this);
  m_humanLabel = new QLabel(this);
  m_epochLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(m_epochLabel);
  layout->addWidget(m_humanLabel);
  setLayout(layout);

  // Periodically refresh the human-readable display so the peripheral visibly
  // ticks. This is purely cosmetic; the register values are sampled live on
  // read, independently of this timer.
  m_displayTimer = new QTimer(this);
  m_displayTimer->setInterval(100);
  connect(m_displayTimer, &QTimer::timeout, this, &IOClock::updateDisplay);
  m_displayTimer->start();
  updateDisplay();
}

uint64_t IOClock::nowNanos() {
  return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());
}

void IOClock::updateDisplay() {
  const uint64_t ns = nowNanos();
  m_epochLabel->setText("Epoch: " + QString::number(ns) + " ns");
  const auto ms = static_cast<qint64>(ns / 1000000ull);
  m_humanLabel->setText(
      QDateTime::fromMSecsSinceEpoch(ms).toString("yyyy-MM-dd hh:mm:ss.zzz"));
}

QString IOClock::description() const {
  QStringList desc;
  desc << "A real-time clock exposing the host machine's wall-clock time as "
          "nanoseconds since the Unix epoch (a 64-bit value).";
  desc << "";
  desc << "The value is split across two 32-bit registers:";
  desc << "  TIME_LO (offset 0): low 32 bits";
  desc << "  TIME_HI (offset 4): high 32 bits";
  desc << "";
  desc
      << "Reading TIME_LO samples the full 64-bit time, returns its low word, "
         "and latches the high word. Read TIME_LO first, then TIME_HI, to "
         "obtain a coherent 64-bit value without tearing. On 64-bit targets "
         "the whole value may be read with a single 8-byte access at offset 0.";
  return desc.join('\n');
}

VInt IOClock::ioRead(AInt offset, unsigned size) {
  // A full-width (>= 8 byte) access returns the whole 64-bit value atomically;
  // no latching is required.
  if (size >= 8) {
    return static_cast<VInt>(nowNanos());
  }

  if (offset >= TIME_HI * 4) {
    // High-word read: return the value latched by the most recent TIME_LO read.
    return m_latchedHigh;
  }

  // Low-word read: sample the full 64-bit time, latch its high word for a
  // subsequent TIME_HI read, and return the low word.
  const uint64_t ns = nowNanos();
  m_latchedHigh = static_cast<uint32_t>(ns >> 32);
  return static_cast<VInt>(static_cast<uint32_t>(ns & 0xFFFFFFFFull));
}

void IOClock::ioWrite(AInt, VInt, unsigned) {
  // Read-only peripheral.
}

} // namespace Ripes
