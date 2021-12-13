#include "ioexternalbus.h"
#include "ui_ioexternalbus.h"

#include <QPainter>
#include <QPen>

#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include "STLExtras.h"
#include "ioregistry.h"

#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <qt5/QtCore/qmutex.h>
#endif

namespace Ripes {

IOExternalBus::IOExternalBus(QWidget* parent)
    : IOBase(IOType::EXTERNALBUS, parent), m_ui(new Ui::IOExternalBus), tcpSocket(new XTcpSocket()) {
    ByteSize = 1;
    skt_use.unlock();

    m_ui->setupUi(this);
    connect(m_ui->connectButton, &QPushButton::clicked, this, &IOExternalBus::connectButtonTriggered);
}

IOExternalBus::~IOExternalBus() {
    unregister();
    delete m_ui;
};

unsigned IOExternalBus::byteSize() const {
    return ByteSize;
}

QString IOExternalBus::description() const {
    return "An external bus is a memory mapped bus handled through network transactions. The peripheral connects to an "
           "IP address denoting a peripheral server - for more details, refer to the Ripes wiki.";
}

VInt IOExternalBus::ioRead(AInt offset, unsigned size) {
    uint32_t rvalue = 0;

    if (tcpSocket->isOpen()) {
        skt_use.lock();
        uint32_t payload = htonl(offset);
        QByteArray dp = QByteArray(reinterpret_cast<const char*>(&payload), 4);
        if (send_cmd(VBUS::VB_PREAD, 4, dp) < 0) {
            skt_use.unlock();
            return 0;
        }

        VBUS::CmdHeader cmd_header;

        if (recv_cmd(cmd_header) < 0) {
            skt_use.unlock();
            return 0;
        }

        if (cmd_header.payload_size) {
            dp = QByteArray(8, 0);
            if (recv_payload(dp, 8) < 0) {
                skt_use.unlock();
                return 0;
            }
            uint32_t* payload = reinterpret_cast<uint32_t*>(dp.data());

            for (uint32_t i = 0; i < 2; i++) {
                payload[i] = ntohl(payload[i]);
            }
            rvalue = payload[1];
        } else {
            disconnectOnError("read error");
        }
        skt_use.unlock();
    }
    return rvalue;
}

void IOExternalBus::ioWrite(AInt offset, VInt value, unsigned size) {
    if (tcpSocket->isOpen()) {
        skt_use.lock();
        uint32_t payload[2];
        payload[0] = htonl(offset);
        payload[1] = htonl(value);
        QByteArray dp = QByteArray(reinterpret_cast<char*>(payload), 8);
        if (send_cmd(VBUS::VB_PWRITE, 8, dp) < 0) {
            skt_use.unlock();
            return;
        }

        VBUS::CmdHeader cmd_header;

        if (recv_cmd(cmd_header) < 0) {
            skt_use.unlock();
            return;
        }

        if (cmd_header.msg_type != VBUS::VB_PWRITE) {
            disconnectOnError("write error");
        }
        skt_use.unlock();
    }
}

void IOExternalBus::connectButtonTriggered() {
    if (!Connected) {
        tcpSocket->abort();
        if (tcpSocket->connectToHost(m_ui->address->text(), m_ui->port->value())) {
            if (send_cmd(VBUS::VB_PINFO) < 0) {
                disconnectOnError();
                return;
            }
            VBUS::CmdHeader cmd_header;

            if (recv_cmd(cmd_header) < 0) {
                disconnectOnError();
                return;
            }
            if (cmd_header.payload_size) {
                QByteArray buff(cmd_header.payload_size + 1, 0);
                if (recv_payload(buff, cmd_header.payload_size) < 0) {
                    disconnectOnError();
                    return;
                }
                QJsonParseError error;
                QJsonDocument desc = QJsonDocument::fromJson(buff.data(), &error);
                if (error.error == QJsonParseError::NoError) {
                    const unsigned int addrw = desc.object().value(QString("address width")).toInt();
                    QJsonObject osymbols = desc.object().value(QString("symbols")).toObject();

                    m_regDescs.clear();
                    for (QJsonObject::iterator i = osymbols.begin(); i != osymbols.end(); i++) {
                        m_regDescs.push_back(
                            RegDesc{i.key(), RegDesc::RW::RW, addrw * 8, static_cast<AInt>(i.value().toInt()), true});
                    }
                    ByteSize = addrw * osymbols.count();
                    updateConnectionStatus(true, desc.object().value(QString("name")).toString());

                } else {
                    QMessageBox::information(nullptr, tr("Ripes VBus"), QString("json: ") + error.errorString());
                    updateConnectionStatus(false);
                    send_cmd(VBUS::VB_QUIT);
                    tcpSocket->abort();
                }
            }
        } else {
            disconnectOnError();
        }
    } else {  // disconnect
        updateConnectionStatus(false);
        send_cmd(VBUS::VB_QUIT);
        tcpSocket->abort();
        m_regDescs.clear();
    }
    emit regMapChanged();
    emit sizeChanged();
}

void IOExternalBus::updateConnectionStatus(bool connected, QString Server) {
    Connected = connected;
    if (Connected) {
        m_ui->connectButton->setText("Disconnect");
        m_ui->status->setText("Connected");
        m_ui->server->setText(Server);
    } else {
        m_ui->connectButton->setText("Connect");
        m_ui->status->setText("Disconnected");
        m_ui->server->setText(Server);
    }
}

int32_t IOExternalBus::send_cmd(const uint32_t cmd, const uint32_t payload_size, const QByteArray& payload) {
    int32_t ret = -1;
    VBUS::CmdHeader cmd_header;

    cmd_header.msg_type = htonl(cmd);
    cmd_header.payload_size = htonl(payload_size);

    QByteArray dp = QByteArray(reinterpret_cast<const char*>(&cmd_header), sizeof(VBUS::CmdHeader));

    if (payload_size) {
        dp.append(payload);
    }

    if ((ret = tcpSocket->write(dp, dp.size())) < 0) {
        disconnectOnError();
        return -1;
    }

    return ret;
}

int32_t IOExternalBus::recv_cmd(VBUS::CmdHeader& cmd_header) {
    QByteArray dp = QByteArray(reinterpret_cast<char*>(&cmd_header), sizeof(VBUS::CmdHeader));
    int ret = tcpSocket->read(dp, sizeof(VBUS::CmdHeader));
    if (ret < 0) {
        disconnectOnError();
        return -1;
    }

    VBUS::CmdHeader* hr = reinterpret_cast<VBUS::CmdHeader*>(dp.data());

    cmd_header.msg_type = ntohl(hr->msg_type);
    cmd_header.payload_size = ntohl(hr->payload_size);

    return ret;
}

int32_t IOExternalBus::recv_payload(QByteArray& buff, const uint32_t payload_size) {
    int ret = tcpSocket->read(buff, payload_size);
    if (ret < 0) {
        disconnectOnError();
        return -1;
    }
    return ret;
}

void IOExternalBus::disconnectOnError(QString msg) {
    skt_use.unlock();

    if (msg.length() > 1) {
        QMessageBox::information(nullptr, tr("Ripes VBus"), msg);
    } else {
        QMessageBox::information(nullptr, tr("Ripes VBus"), tcpSocket->getLastErrorStr());
    }
    tcpSocket->close();
    updateConnectionStatus(false);
    tcpSocket->abort();
    m_regDescs.clear();

    emit regMapChanged();
    emit sizeChanged();
}

}  // namespace Ripes
