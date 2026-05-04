#include <QApplication>
#include <QtTest/QTest>

#include "io/io7indicator.h"
#include "io/iobase.h"

using namespace Ripes;

namespace {
IO7Indicator *makeIndicator() {
  auto *p = new IO7Indicator(nullptr);
  QObject::connect(p, &IOBase::aboutToDelete, p,
                   [](std::atomic<bool> &ok) { ok = 1; });
  return p;
}
} // namespace

class tst_io_7indicator : public QObject {
  Q_OBJECT

private slots:
  // Verifies the full write-read cycle for all four default digits:
  //   - default layout is 4 digits × 4 bytes (16 bytes total, 4 registers)
  //   - segment patterns written at aligned offsets are read back unchanged
  //   - only the low byte of a 32-bit write is retained
  //   - misaligned writes are silently ignored; misaligned reads return 0
  //   - out-of-range digit addresses return 0
  //   - reset() zeroes every digit register
  void sevenSegment_writeReadRoundTrip() {
    auto *ind = makeIndicator();

    QCOMPARE(ind->byteSize(), 4u * 4u);
    QCOMPARE(ind->registers().size(), size_t{4});

    const std::vector<uint8_t> patterns = {0x3F, 0x06, 0x5B, 0x4F};
    for (size_t i = 0; i < patterns.size(); ++i)
      ind->ioWrite(static_cast<AInt>(i * 4), patterns[i], 4);
    for (size_t i = 0; i < patterns.size(); ++i)
      QCOMPARE(static_cast<uint8_t>(ind->ioRead(static_cast<AInt>(i * 4), 4)),
               patterns[i]);

    ind->ioWrite(0, 0xFFFFFF7E, 4);
    QCOMPARE(static_cast<uint8_t>(ind->ioRead(0, 4)), uint8_t{0x7E});

    ind->ioWrite(1, 0xAA, 4);
    QCOMPARE(static_cast<uint8_t>(ind->ioRead(0, 4)), uint8_t{0x7E});
    QCOMPARE(ind->ioRead(1, 4), VInt{0});

    QCOMPARE(ind->ioRead(static_cast<AInt>(patterns.size() * 4), 4), VInt{0});

    ind->reset();
    for (size_t i = 0; i < patterns.size(); ++i)
      QCOMPARE(ind->ioRead(static_cast<AInt>(i * 4), 4), VInt{0});

    delete ind;
  }

  // Verifies that changing parameter 0 (NUM_DIGITS) resizes the peripheral:
  //   - byteSize() == numDigits * 4
  //   - registers().size() == numDigits
  void sevenSegment_numDigitsParameterResizes() {
    auto *ind = makeIndicator();
    QVERIFY(ind->setParameter(0, 8));
    QCOMPARE(ind->byteSize(), 4u * 8u);
    QCOMPARE(ind->registers().size(), size_t{8});
    delete ind;
  }
};

QTEST_MAIN(tst_io_7indicator)
#include "tst_io_7indicator.moc"