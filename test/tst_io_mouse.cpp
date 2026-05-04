#include <QApplication>
#include <QMouseEvent>
#include <QtTest/QTest>

#include "io/iobase.h"
#include "io/iomouse.h"

using namespace Ripes;

namespace {
IOMouse *makeMouse() {
  auto *p = new IOMouse(nullptr);
  QObject::connect(p, &IOBase::aboutToDelete, p,
                   [](std::atomic<bool> &ok) { ok = 1; });
  return p;
}
} // namespace

class tst_io_mouse : public QObject {
  Q_OBJECT

private slots:
  // Verifies that mouse-move events update X/Y registers and are clamped:
  //   - in-range coordinates are stored exactly
  //   - coordinates outside [0, WIDTH-1] / [0, HEIGHT-1] are clamped to the
  //     nearest boundary (max or 0), not wrapped or rejected
  void mouse_moveClampsAndUpdatesRegisters() {
    auto *m = makeMouse();
    constexpr AInt X = 0;
    constexpr AInt Y = 4;

    QMouseEvent move(QEvent::MouseMove, QPointF(50, 75), QPointF(50, 75),
                     Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(m, &move);
    QCOMPARE(m->ioRead(X, 4), VInt{50});
    QCOMPARE(m->ioRead(Y, 4), VInt{75});

    QMouseEvent moveOOB(QEvent::MouseMove, QPointF(9999, -10),
                        QPointF(9999, -10), Qt::NoButton, Qt::NoButton,
                        Qt::NoModifier);
    QCoreApplication::sendEvent(m, &moveOOB);
    QCOMPARE(m->ioRead(X, 4), VInt{199});
    QCOMPARE(m->ioRead(Y, 4), VInt{0});

    delete m;
  }

  // Verifies button state tracking and the software-writable SCROLL register:
  //   - left-button press sets LBUTTON=1; release sets LBUTTON=0
  //   - right button is tracked independently of the left button
  //   - SCROLL register accepts arbitrary values written by the program
  //   (ack/clear)
  //   - reset() zeroes LBUTTON, RBUTTON, and SCROLL simultaneously
  void mouse_buttonsAndScroll() {
    auto *m = makeMouse();
    constexpr AInt LBUTTON = 8;
    constexpr AInt RBUTTON = 12;
    constexpr AInt SCROLL = 16;
    QPointF p(10, 10);

    QCOMPARE(m->ioRead(LBUTTON, 4), VInt{0});
    QCOMPARE(m->ioRead(RBUTTON, 4), VInt{0});

    QMouseEvent lpress(QEvent::MouseButtonPress, p, p, Qt::LeftButton,
                       Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(m, &lpress);
    QCOMPARE(m->ioRead(LBUTTON, 4), VInt{1});

    QMouseEvent lrelease(QEvent::MouseButtonRelease, p, p, Qt::LeftButton,
                         Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(m, &lrelease);
    QCOMPARE(m->ioRead(LBUTTON, 4), VInt{0});

    QMouseEvent rpress(QEvent::MouseButtonPress, p, p, Qt::RightButton,
                       Qt::RightButton, Qt::NoModifier);
    QCoreApplication::sendEvent(m, &rpress);
    QCOMPARE(m->ioRead(RBUTTON, 4), VInt{1});
    QCOMPARE(m->ioRead(LBUTTON, 4), VInt{0});

    m->ioWrite(SCROLL, 42, 4);
    QCOMPARE(m->ioRead(SCROLL, 4), VInt{42});

    m->reset();
    QCOMPARE(m->ioRead(LBUTTON, 4), VInt{0});
    QCOMPARE(m->ioRead(RBUTTON, 4), VInt{0});
    QCOMPARE(m->ioRead(SCROLL, 4), VInt{0});

    delete m;
  }
};

QTEST_MAIN(tst_io_mouse)
#include "tst_io_mouse.moc"