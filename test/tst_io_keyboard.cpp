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
  void keyboard_keyPressEnqueuesAndStatusReflectsBuffer() {
    auto *kb = makeKeyboard();

    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_STATUS, 4), VInt{0});
    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_DATA, 1), VInt{0});

    const QList<QPair<int, char>> keys = {
        {Qt::Key_A, 'A'}, {Qt::Key_B, 'B'}, {Qt::Key_5, '5'}};

    for (const auto &k : keys) {
      QKeyEvent ev(QEvent::KeyPress, k.first, Qt::NoModifier, QString(k.second));
      QCoreApplication::sendEvent(kb, &ev);
    }

    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_STATUS, 4), VInt{3});

    for (const auto &k : keys)
      QCOMPARE(static_cast<char>(kb->ioRead(IOKeyboard::RegMap::KEY_DATA, 1)), k.second);

    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_DATA, 1), VInt{0});

    QKeyEvent ignored(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier, QString());
    QCoreApplication::sendEvent(kb, &ignored);

    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_STATUS, 4), VInt{0});

    delete kb;
  }

  void keyboard_writeStatusClearsBuffer() {
    auto *kb = makeKeyboard();

    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Q, Qt::NoModifier, "Q");

    QCoreApplication::sendEvent(kb, &ev);
    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_STATUS, 4), VInt{1});

    kb->ioWrite(IOKeyboard::RegMap::KEY_STATUS, 1, 4);
    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_STATUS, 4), VInt{0});

    QCoreApplication::sendEvent(kb, &ev);
    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_STATUS, 4), VInt{1});

    kb->reset();
    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_STATUS, 4), VInt{0});

    delete kb;
  }

  void keyboard_bufferOverflowIsBounded() {
    auto *kb = makeKeyboard();

    kb->setParameter(0, 4);

    QKeyEvent ev(QEvent::KeyPress, Qt::Key_X, Qt::NoModifier, "X");

    for (int i = 0; i < 10; ++i)
      QCoreApplication::sendEvent(kb, &ev);

    QCOMPARE(kb->ioRead(IOKeyboard::RegMap::KEY_STATUS, 4), VInt{4});

    delete kb;
  }
};

QTEST_MAIN(tst_io_keyboard)
#include "tst_io_keyboard.moc"