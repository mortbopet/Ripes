#pragma once

#include <QWidget>
#include <vector>

#include "iobase.h"

namespace Ripes {

class IO7Indicator : public IOBase {
  Q_OBJECT

  enum Parameters { NUM_DIGITS = 0, COLOR = 1 };

public:
  static constexpr unsigned DEFAULT_NUM_DIGITS = 4;
  static constexpr unsigned MIN_NUM_DIGITS = 1;
  static constexpr unsigned MAX_NUM_DIGITS = 16;

  IO7Indicator(QWidget *parent);
  ~IO7Indicator() { unregister(); };

  virtual unsigned byteSize() const override;
  virtual QString description() const override;
  virtual QString baseName() const override { return "Seven Segment"; }

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
  QSize minimumSizeHint() const override;

private:
  void updateRegDescs();
  unsigned numDigits() const;
  void drawDigit(QPainter &p, int x, int y, int w, int h, uint8_t segments);

  std::vector<uint8_t> m_digitValues;
  std::vector<RegDesc> m_regDescs;
  std::vector<IOSymbol> m_extraSymbols;
};

} // namespace Ripes