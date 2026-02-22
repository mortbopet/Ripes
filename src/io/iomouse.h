#pragma once

#include <QPen>
#include <QWidget>

#include "iobase.h"

namespace Ripes {

class IOMouse : public IOBase {
  Q_OBJECT

  enum Regs { X, Y, LBUTTON, RBUTTON, SCROLL };

public:
  IOMouse(QWidget *parent);
  ~IOMouse() { unregister(); }

  unsigned byteSize() const override { return 5 * 4; }
  QString description() const override;
  QString baseName() const override { return "Mouse"; }

  const std::vector<RegDesc> &registers() const override { return m_regDescs; }
  const std::vector<IOSymbol> *extraSymbols() const override { return &m_extraSymbols; }

  VInt ioRead(AInt offset, unsigned size) override;
  void ioWrite(AInt offset, VInt value, unsigned size) override;

  void reset() override;

protected:
  void parameterChanged(unsigned) override {}

  void paintEvent(QPaintEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  QSize minimumSizeHint() const override;

private:

  int m_mouseX = 0;
  int m_mouseY = 0;
  int m_lButton = 0;
  int m_rButton = 0;
  int m_scroll = 0;

  unsigned m_width = 350;
  unsigned m_height = 300;

  std::vector<RegDesc> m_regDescs;
  std::vector<IOSymbol> m_extraSymbols;
};
} // namespace Ripes

