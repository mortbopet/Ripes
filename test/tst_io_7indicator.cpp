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
  // Verifies the full write-read cycle for the default digit layout:
  //   - byteSize() and registers().size() match numDigits()
  //   - segment patterns written at aligned offsets are read back unchanged
  //   - only the low byte of a 32-bit write is retained
  //   - misaligned writes and invalid sizes are silently ignored
  //   - misaligned and out-of-range reads return 0
  //   - reset() zeroes every digit register
  void sevenSegment_writeReadRoundTrip() {
    auto *ind = makeIndicator();
    const unsigned digits = ind->numDigits();
    QCOMPARE(ind->byteSize(), digits * 4u);
    QCOMPARE(ind->registers().size(), size_t{digits});

    std::vector<uint8_t> patterns;
    patterns.reserve(digits);
    const std::vector<uint8_t> seed = {0x3F, 0x06, 0x5B, 0x4F, 0x66,
                                       0x6D, 0x7D, 0x07, 0x7F, 0x6F};
    for (unsigned i = 0; i < digits; ++i)
      patterns.push_back(seed[i % seed.size()]);

    for (size_t i = 0; i < patterns.size(); ++i)
      ind->ioWrite(static_cast<AInt>(i * 4), patterns[i], 4);

    for (size_t i = 0; i < patterns.size(); ++i)
      QCOMPARE(static_cast<uint8_t>(ind->ioRead(static_cast<AInt>(i * 4), 4)),
               patterns[i]);

    // Check truncation to 8 bits
    ind->ioWrite(0, 0xFFFFFF7E, 4);
    QCOMPARE(static_cast<uint8_t>(ind->ioRead(0, 4)), uint8_t{0x7E});

    // Misaligned writes should be ignored
    ind->ioWrite(1, 0xAA, 4);
    QCOMPARE(static_cast<uint8_t>(ind->ioRead(0, 4)), uint8_t{0x7E});
    QCOMPARE(ind->ioRead(1, 4), VInt{0});

    // Invalid write sizes should be ignored (only size 4 is allowed)
    ind->ioWrite(0, 0xBB, 1);
    ind->ioWrite(0, 0xBBBB, 2);
    QCOMPARE(static_cast<uint8_t>(ind->ioRead(0, 4)), uint8_t{0x7E});

    // Out of range reads
    QCOMPARE(ind->ioRead(static_cast<AInt>(patterns.size() * 4), 4), VInt{0});
    QCOMPARE(ind->ioRead(static_cast<AInt>(-4), 4), VInt{0});

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
    QCOMPARE(ind->numDigits(), 8u);
    QCOMPARE(ind->byteSize(), 4u * 8u);
    QCOMPARE(ind->registers().size(), size_t{8});
    delete ind;
  }

  // Verifies correct handling of parameter constraints and color updates
  void sevenSegment_parameterBounds() {
    auto *ind = makeIndicator();

    // Verify minimum and maximum digit bounds
    QVERIFY(ind->setParameter(0, IO7Indicator::MAX_NUM_DIGITS));
    QCOMPARE(ind->numDigits(), IO7Indicator::MAX_NUM_DIGITS);

    QVERIFY(ind->setParameter(0, IO7Indicator::MIN_NUM_DIGITS));
    QCOMPARE(ind->numDigits(), IO7Indicator::MIN_NUM_DIGITS);

    // Verify COLOR parameter update
    QVERIFY(ind->setParameter(1, 2)); 

    delete ind;
  }
};

QTEST_MAIN(tst_io_7indicator)
#include "tst_io_7indicator.moc"