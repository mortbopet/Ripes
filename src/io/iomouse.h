#pragma once

#include <QWidget>

#include "iobase.h"

class QSpinBox;
class QPushButton;

namespace Ripes {

class IOMouse : public IOBase {
  Q_OBJECT

  enum Regs { X, Y, LBUTTON, RBUTTON, SCROLL };
  enum Parameters { WIDTH, HEIGHT };

public:
  IOMouse(QWidget *parent);
  ~IOMouse() { unregister(); };

  virtual unsigned byteSize() const override { return 5 * 4; }
  virtual QString description() const override;
  virtual QString baseName() const override { return "Mouse"; }

  virtual const std::vector<RegDesc> &registers() const override {
    return m_regDescs;
  };
  virtual const std::vector<IOSymbol> *extraSymbols() const override {
    return &m_extraSymbols;
  }

  virtual VInt ioRead(AInt offset, unsigned size) override;
  virtual void ioWrite(AInt offset, VInt value, unsigned size) override;

  virtual void reset() override;

protected:
  virtual void parameterChanged(unsigned) override;

  void paintEvent(QPaintEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

private:
  void updateExtraSymbols();

  int m_mouseX = 0;
  int m_mouseY = 0;
  int m_lButton = 0;
  int m_rButton = 0;
  int m_mButton = 0;
  int m_scroll = 0;

  std::vector<RegDesc> m_regDescs;
  std::vector<IOSymbol> m_extraSymbols;
};
} // namespace Ripes