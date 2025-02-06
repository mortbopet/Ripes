#pragma once

#include <QMutex>
#include <QPen>
#include <QVariant>

#include "iobase.h"

#include "VBus.h"
#include "XTcpSocket.h"

namespace Ripes {

namespace Ui {
class IOExternalBus;
}

/// @todo: should the bus be discoverable? i.e. implement a protocol for
/// external bus to provide info on symbols and registers exposed by its
/// peripherals, which will be made available in Ripes. This will also
/// dynamically adjust the "byteSize" return value.

class IOExternalBus : public IOBase {
  Q_OBJECT

public:
  IOExternalBus(QWidget *parent);
  ~IOExternalBus() override;

  virtual unsigned byteSize() const override;
  virtual QString description() const override;
  virtual QString baseName() const override { return "External bus"; }

  virtual const std::vector<RegDesc> &registers() const override {
    return m_regDescs;
  };
  virtual const std::vector<IOSymbol> *extraSymbols() const override {
    return &m_extraSymbols;
  }

  /**
   * Hardware read/write functions
   */
  virtual VInt ioRead(AInt offset, unsigned size) override;
  virtual void ioWrite(AInt offset, VInt value, unsigned size) override;

protected:
  virtual void parameterChanged(unsigned) override{/* no parameters */};

private slots:
  void connectButtonTriggered();

private:
  bool m_Connected = false;
  uint32_t m_ByteSize;
  int32_t send_cmd(const uint32_t cmd, const uint32_t payload_size,
                   const QByteArray &payload, const uint64_t time);
  int32_t recv_cmd(VBUS::CmdHeader &cmd_header);
  int32_t recv_payload(QByteArray &buff, const uint32_t payload_size);
  void disconnectOnError(const QString msg = "");
  void updateConnectionStatus(bool connected, QString Server = "-");
  void updateAddress();
  Ui::IOExternalBus *m_ui = nullptr;

  std::vector<RegDesc> m_regDescs;
  std::vector<IOSymbol> m_extraSymbols;

  std::unique_ptr<XTcpSocket> tcpSocket = nullptr;
  QMutex skt_use;
};
} // namespace Ripes
