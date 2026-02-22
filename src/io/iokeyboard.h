#pragma once
#include <QGridLayout>
#include <QLabel>
#include <QMutex>
#include <QPushButton>
#include <QQueue>
#include <QWidget>
#include "iobase.h"

namespace Ripes {

class IOKeyboard : public IOBase {
  Q_OBJECT

public:
  IOKeyboard(QWidget *parent);
  ~IOKeyboard() { unregister(); }

  unsigned byteSize() const override { return 8; }
  QString description() const override { return QString(); }
  QString baseName() const override { return "Keyboard"; }

  const std::vector<RegDesc> &registers() const override {
    return m_regDescs;
  }
  const std::vector<IOSymbol> *extraSymbols() const override {
    return nullptr;
  }

  VInt ioRead(AInt offset, unsigned size) override;
  void ioWrite(AInt offset, VInt value, unsigned size) override;
  void reset() override;

protected:
  void keyPressEvent(QKeyEvent *event) override;

private:
  void updateLayout();
  void enqueueKey(uint8_t ascii);
  void refreshStatusLabel();

  QQueue<uint8_t> m_keyBuffer;
  mutable QMutex m_bufMutex;

  QGridLayout *m_mainLayout = nullptr;
  QLabel *m_statusLabel = nullptr;

  std::vector<RegDesc> m_regDescs;
};

}