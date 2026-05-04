#include <QApplication>
#include <QKeyEvent>
#include <QtTest/QTest>

#include "io/iobase.h"
#include "io/iokeyboard.h"

using namespace Ripes;

namespace {
IOKeyboard *makeKeyboard() {
  auto *p = new IOKeyboard(nullptr);
  QObject::connect(p, &IOBase::aboutToDelete, p,
                   [](std::atomic<bool> &ok) { ok = 1; });
  return p;
}
} // namespace

class tst_io_keyboard : public QObject {
  Q_OBJECT

private slots:
  // Verifies FIFO enqueue/dequeue behaviour and KEY_STATUS accounting:
  //   - empty buffer: KEY_STATUS == 0, KEY_DATA read returns 0
  //   - real QKeyEvent presses are forwarded and counted in KEY_STATUS
  //   - KEY_DATA reads dequeue keys in FIFO order
  //   - after the buffer is drained, KEY_DATA returns 0
  //   - unmapped keys (e.g. F1) are not enqueued
  void keyboard_keyPressEnqueuesAndStatusReflectsBuffer() {
    auto *kb = makeKeyboard();
    constexpr AInt KEY_DATA = 0;
    constexpr AInt KEY_STATUS = 4;

    QCOMPARE(kb->ioRead(KEY_STATUS, 4), VInt{0});
    QCOMPARE(kb->ioRead(KEY_DATA, 1), VInt{0});

    const QList<QPair<int, char>> keys = {
        {Qt::Key_A, 'A'}, {Qt::Key_B, 'B'}, {Qt::Key_5, '5'}};
    for (const auto &k : keys) {
      QKeyEvent ev(QEvent::KeyPress, k.first, Qt::NoModifier);
      QCoreApplication::sendEvent(kb, &ev);
    }
    QCOMPARE(kb->ioRead(KEY_STATUS, 4), VInt{3});

    for (const auto &k : keys)
      QCOMPARE(static_cast<char>(kb->ioRead(KEY_DATA, 1)), k.second);
    QCOMPARE(kb->ioRead(KEY_DATA, 1), VInt{0});

    QKeyEvent ignored(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
    QCoreApplication::sendEvent(kb, &ignored);
    QCOMPARE(kb->ioRead(KEY_STATUS, 4), VInt{0});

    delete kb;
  }

  // Verifies that the key buffer can be cleared in two ways:
  //   - a non-zero write to KEY_STATUS flushes the FIFO
  //   - reset() also flushes the FIFO
  void keyboard_writeStatusClearsBuffer() {
    auto *kb = makeKeyboard();
    constexpr AInt KEY_STATUS = 4;

    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Q, Qt::NoModifier);
    QCoreApplication::sendEvent(kb, &ev);
    QCOMPARE(kb->ioRead(KEY_STATUS, 4), VInt{1});

    kb->ioWrite(KEY_STATUS, 1, 4);
    QCOMPARE(kb->ioRead(KEY_STATUS, 4), VInt{0});

    QCoreApplication::sendEvent(kb, &ev);
    QCOMPARE(kb->ioRead(KEY_STATUS, 4), VInt{1});
    kb->reset();
    QCOMPARE(kb->ioRead(KEY_STATUS, 4), VInt{0});

    delete kb;
  }

  // Verifies that the FIFO is bounded by the BUFSIZE parameter:
  //   - excess events beyond the configured capacity are silently dropped
  //   - KEY_STATUS never exceeds BUFSIZE regardless of how many events arrive
  void keyboard_bufferOverflowIsBounded() {
    auto *kb = makeKeyboard();
    constexpr AInt KEY_STATUS = 4;
    kb->setParameter(0, 4);

    QKeyEvent ev(QEvent::KeyPress, Qt::Key_X, Qt::NoModifier);
    for (int i = 0; i < 10; ++i)
      QCoreApplication::sendEvent(kb, &ev);
    QCOMPARE(kb->ioRead(KEY_STATUS, 4), VInt{4});

    delete kb;
  }
};

QTEST_MAIN(tst_io_keyboard)
#include "tst_io_keyboard.moc"