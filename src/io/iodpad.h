#pragma once

#include <QPen>
#include <QVariant>
#include <QWidget>

#include <array>
#include <atomic>

QT_FORWARD_DECLARE_CLASS(QAbstractButton);

#include "iobase.h"

namespace Ripes {

class IODPad : public IOBase {
  Q_OBJECT

  enum IdxToDir { UP, DOWN, LEFT, RIGHT, DIRECTIONS };

public:
  IODPad(QWidget *parent);
  ~IODPad() { unregister(); };

  virtual unsigned byteSize() const override;
  virtual QString description() const override;
  virtual QString baseName() const override { return "D-Pad"; };

  virtual const std::vector<RegDesc> &registers() const override {
    return m_regDescs;
  };

  /**
   * Hardware read/write functions
   */
  virtual VInt ioRead(AInt offset, unsigned size) override;
  virtual void ioWrite(AInt offset, VInt value, unsigned size) override;

  virtual void reset() override;

protected:
  virtual void parameterChanged(unsigned) override { /* no parameters */ };
  void keyPressEvent(QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;
  void mousePressEvent(QMouseEvent *e) override;

private:
  /// Maps a key code (WASD or arrow keys) to a direction. Returns true and sets
  /// @p dir on a match.
  bool keyToDirection(int key, IdxToDir &dir) const;

  /// Updates the pressed state of a direction (from keyboard or mouse),
  /// updating the button visual, the current-state bit, and - on a press - the
  /// "pressed since last read" latch.
  void setDirection(IdxToDir dir, bool pressed);

  constexpr static unsigned m_maxSideWidth = 256;
  std::vector<RegDesc> m_regDescs;
  std::map<IdxToDir, QAbstractButton *> m_buttons;

  // Input state shared between the GUI thread (which updates it from key/mouse
  // events) and the processor thread (which reads it via ioRead during a run),
  // hence the atomics.
  //
  // m_down    : the current physical (held) state of each direction.
  // m_latched : set when a direction is pressed, cleared when read. This edge
  //             capture ensures that a quick tap occurring entirely between two
  //             polls is still observed once by the program (modelling a simple
  //             latching/edge-triggered input register rather than a purely
  //             level-sensitive one).
  std::array<std::atomic<bool>, DIRECTIONS> m_down;
  std::array<std::atomic<bool>, DIRECTIONS> m_latched;
};
} // namespace Ripes
