#pragma once

#include <QPen>
#include <QVariant>
#include <QWidget>

#include "iobase.h"

#include "VBus.h"
#include "XTcpSocket.h"

namespace Ripes {

namespace Ui {
class IOExternalBus;
}

/// @todo: should the bus be discoverable? i.e. implement a protocol for external bus to provide info on symbols and
/// registers exposed by its peripherals, which will be made available in Ripes. This will also dynamically adjust the
/// "byteSize" return value.

class IOExternalBus : public IOBase {
    Q_OBJECT

public:
    IOExternalBus(QWidget* parent);
    ~IOExternalBus() override;

    virtual unsigned byteSize() const override;
    virtual QString description() const override;
    virtual QString baseName() const override { return "External bus"; }

    virtual const std::vector<RegDesc>& registers() const override { return m_regDescs; };
    virtual const std::vector<IOSymbol>* extraSymbols() const override { return &m_extraSymbols; }

    /**
     * Hardware read/write functions
     */
    virtual VInt ioRead(AInt offset, unsigned size) override;
    virtual void ioWrite(AInt offset, VInt value, unsigned size) override;

protected:
    virtual void parameterChanged(unsigned) override;

private slots:
    void connectButtonTriggered();
    // void sktConnected();
    // void sktRead();

private:
    uint32_t ByteSize;
    int32_t send_cmd(const uint32_t cmd, const uint32_t payload_size = 0, const char* payload = nullptr);
    int32_t recv_cmd(cmd_header_t* cmd_header);
    int32_t recv_payload(char* buff, const uint32_t payload_size);

    void updateAddress();
    Ui::IOExternalBus* m_ui = nullptr;

    std::vector<RegDesc> m_regDescs;
    std::vector<IOSymbol> m_extraSymbols;

    XTcpSocket* tcpSocket = nullptr;
    unsigned char skt_use;
};
}  // namespace Ripes
