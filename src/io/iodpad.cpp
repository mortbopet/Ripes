#include "iodpad.h"
#include "ioregistry.h"

#include <QAbstractButton>
#include <QGridLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QToolButton>

namespace Ripes {

IODPad::IODPad(QWidget *parent) : IOBase(IOType::DPAD, parent) {
  for (auto &b : m_down)
    b.store(false);
  for (auto &b : m_latched)
    b.store(false);

  for (unsigned i = 0; i < DIRECTIONS; ++i) {
    QString name;
    Qt::ArrowType arrow;
    switch (i) {
    case UP:
      name = "UP";
      arrow = Qt::UpArrow;
      break;
    case DOWN:
      name = "DOWN";
      arrow = Qt::DownArrow;
      break;
    case LEFT:
      name = "LEFT";
      arrow = Qt::LeftArrow;
      break;
    case RIGHT:
      name = "RIGHT";
      arrow = Qt::RightArrow;
      break;
    }
    auto *button = new QToolButton();
    m_buttons[static_cast<IdxToDir>(i)] = button;
    button->setArrowType(arrow);
    // Keep keyboard focus on the D-pad container (so WASD/arrow keys reach its
    // key handlers) rather than letting a button steal it.
    button->setFocusPolicy(Qt::NoFocus);

    // Drive the input state from mouse interaction with the button.
    const auto dir = static_cast<IdxToDir>(i);
    connect(button, &QAbstractButton::pressed, this, [this, dir] {
      setFocus(Qt::MouseFocusReason);
      setDirection(dir, true);
    });
    connect(button, &QAbstractButton::released, this,
            [this, dir] { setDirection(dir, false); });

    m_regDescs.push_back(RegDesc{name, RegDesc::RW::R, 1, i * 4, true});
  }

  auto *gridLayout = new QGridLayout();
  gridLayout->addWidget(m_buttons[UP], 0, 1);
  gridLayout->addWidget(m_buttons[DOWN], 2, 1);
  gridLayout->addWidget(m_buttons[LEFT], 1, 0);
  gridLayout->addWidget(m_buttons[RIGHT], 1, 2);

  setLayout(gridLayout);

  // Accept keyboard focus so that WASD/arrow key presses are delivered to this
  // widget's key handlers once it has been clicked/highlighted.
  setFocusPolicy(Qt::StrongFocus);
}

unsigned IODPad::byteSize() const { return 4 * 4; }

bool IODPad::keyToDirection(int key, IdxToDir &dir) const {
  switch (key) {
  case Qt::Key_W:
  case Qt::Key_Up:
    dir = UP;
    return true;
  case Qt::Key_S:
  case Qt::Key_Down:
    dir = DOWN;
    return true;
  case Qt::Key_A:
  case Qt::Key_Left:
    dir = LEFT;
    return true;
  case Qt::Key_D:
  case Qt::Key_Right:
    dir = RIGHT;
    return true;
  }
  return false;
}

void IODPad::setDirection(IdxToDir dir, bool pressed) {
  m_down[dir].store(pressed, std::memory_order_relaxed);
  if (pressed) {
    // Latch the press so that even a tap shorter than the program's polling
    // interval is observed once (cleared on read in ioRead).
    m_latched[dir].store(true, std::memory_order_relaxed);
  }
  auto it = m_buttons.find(dir);
  if (it != m_buttons.end())
    it->second->setDown(pressed);
}

void IODPad::mousePressEvent(QMouseEvent *e) {
  // Ensure clicking anywhere on the D-pad gives it keyboard focus, so the
  // subsequent WASD/arrow keys are delivered here.
  setFocus(Qt::MouseFocusReason);
  IOBase::mousePressEvent(e);
}

void IODPad::keyPressEvent(QKeyEvent *e) {
  IdxToDir dir;
  if (keyToDirection(e->key(), dir)) {
    // Auto-repeat presses simply re-assert the held state, which is fine.
    setDirection(dir, true);
    e->accept();
    return;
  }
  IOBase::keyPressEvent(e);
}

void IODPad::keyReleaseEvent(QKeyEvent *e) {
  IdxToDir dir;
  if (keyToDirection(e->key(), dir)) {
    // Ignore synthetic auto-repeat releases (generated while a key is held on
    // some platforms) so a held direction does not flicker off between polls.
    if (e->isAutoRepeat()) {
      e->accept();
      return;
    }
    setDirection(dir, false);
    e->accept();
    return;
  }
  IOBase::keyReleaseEvent(e);
}

QString IODPad::description() const {
  QStringList desc;
  desc << "Each button maps to a 32-bit register, with the least-significant "
          "bit indicating the state of the "
          "button.\n";
  desc << "If the D-pad window is in focus, the buttons may be pressed using "
          "the \"WASD\" or arrow keys of the keyboard.\n";
  desc << "A button register reads as 1 if the button is currently held, or if "
          "it was pressed at any point since the register was last read "
          "(edge/press capture). Reading a register clears this captured "
          "press, so brief taps between polls are not missed.";

  return desc.join('\n');
}

VInt IODPad::ioRead(AInt offset, unsigned) {
  const unsigned idx = static_cast<unsigned>(offset) / 4;
  if (idx >= DIRECTIONS)
    return 0;
  const bool down = m_down[idx].load(std::memory_order_relaxed);
  // Read-to-clear the captured press.
  const bool latched =
      m_latched[idx].exchange(false, std::memory_order_relaxed);
  return (down || latched) ? 1 : 0;
}

void IODPad::ioWrite(AInt, VInt, unsigned) {
  // Write-only
}

void IODPad::reset() {
  for (unsigned i = 0; i < DIRECTIONS; ++i) {
    m_down[i].store(false, std::memory_order_relaxed);
    m_latched[i].store(false, std::memory_order_relaxed);
    auto it = m_buttons.find(static_cast<IdxToDir>(i));
    if (it != m_buttons.end())
      it->second->setDown(false);
  }
}

} // namespace Ripes
