#pragma once

#include <QMutex>
#include <QQueue>
#include <QWidget>
#include <optional>
#include "iobase.h"

QT_FORWARD_DECLARE_CLASS(QLabel);
QT_FORWARD_DECLARE_CLASS(QPushButton);

namespace Ripes {

class IOKeyboard : public IOBase {
  Q_OBJECT

  enum Parameters { BUFSIZE };

  enum RegMap : AInt {
    KEY_DATA = 0x0,
    KEY_STATUS = 0x4
  };

public:
  IOKeyboard(QWidget *parent);
  ~IOKeyboard() { unregister(); };

  virtual unsigned byteSize() const override { return 8; }
  virtual QString description() const override;
  virtual QString baseName() const override { return "Keyboard"; }

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
  void keyPressEvent(QKeyEvent *event) override;

private:
  void buildLayout();
  void refreshRegMap();
  void refreshStatusLabel();
  void enqueueKey(uint8_t ascii);

  QQueue<uint8_t> m_keyBuffer;
  mutable QMutex m_bufMutex;
  std::optional<uint8_t> m_lastKey;

  QLabel *m_statusLabel = nullptr;

  std::vector<RegDesc> m_regDescs;
  std::vector<IOSymbol> m_extraSymbols;
};

} // namespace Ripes