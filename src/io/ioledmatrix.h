#pragma once

#include <QPen>
#include <QVariant>
#include <QWidget>

#include "iobase.h"

namespace Ripes {

class IOLedMatrix : public IOBase {
  Q_OBJECT

  enum Parameters { HEIGHT, WIDTH, SIZE };

public:
  IOLedMatrix(QWidget *parent);
  ~IOLedMatrix() override { unregister(); };

  unsigned byteSize() const override;
  QString description() const override;
  QString baseName() const override { return "LED Matrix"; }

  const std::vector<RegDesc> &registers() const override { return m_regDescs; };
  const std::vector<IOSymbol> *extraSymbols() const override {
    return &m_extraSymbols;
  }

  /**
   * Hardware read/write functions
   */
  VInt ioRead(AInt offset, unsigned size) override;
  void ioWrite(AInt offset, VInt value, unsigned size) override;

  void reset() override { std::fill(m_ledRegs.begin(), m_ledRegs.end(), 0); }

protected:
  void parameterChanged(unsigned) override { updateLEDRegs(); };

  /**
   * QWidget drawing
   */
  void paintEvent(QPaintEvent *event) override;
  QSize minimumSizeHint() const override;

private:
  VInt regRead(AInt offset) const;
  void updateLEDRegs();

  unsigned m_maxSideWidth = 256;
  std::vector<uint32_t> m_ledRegs;
  std::vector<RegDesc> m_regDescs;
  std::vector<IOSymbol> m_extraSymbols;

  QPen m_pen;
};
} // namespace Ripes
