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
#endif

#define dprintf \
    if (1) {    \
    } else      \
        printf

namespace Ripes {

IOExternalBus::IOExternalBus(QWidget* parent)
    : IOBase(IOType::EXTERNALBUS, parent), m_ui(new Ui::IOExternalBus), tcpSocket(new XTcpSocket()) {
    ByteSize = 1;
    skt_use = 0;

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
        while (skt_use)
            QApplication::processEvents();  // TODO only need in rvss, change to mutex
        skt_use = 1;
        uint32_t payload = htonl(offset);
        QByteArray dp = QByteArray(static_cast<const char*>((void*)&payload), 4);
        if (send_cmd(VB_PREAD, 4, &dp) < 0) {
            skt_use = 0;
            return 0;
        }

        cmd_header_t cmd_header;

        if (recv_cmd(cmd_header) < 0) {
            skt_use = 0;
            return 0;
        }

        if (cmd_header.payload_size) {
            dp = QByteArray(8, 0);
            if (recv_payload(dp, 8) < 0) {
                skt_use = 0;
                return 0;
            }
            uint32_t* payload = (uint32_t*)dp.data();

            for (uint32_t i = 0; i < 2; i++) {
                payload[i] = ntohl(payload[i]);
            }
            dprintf("Read addr[%x] = %x \n", payload[0], payload[1]);
            rvalue = payload[1];
        } else {
            skt_use = 0;
            printf("read error [%lx] (%x) msg=%i pyaload_size=%i !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", offset, size,
                   cmd_header.msg_type, cmd_header.payload_size);
            tcpSocket->close();
        }
        skt_use = 0;
    }

    dprintf("===============>ioRead [%lx] = %x (%x)\n", offset, rvalue, size);
    return rvalue;
}

void IOExternalBus::ioWrite(AInt offset, VInt value, unsigned size) {
    dprintf("===============>ioWrite [%lx] = %lx (%x)\n", offset, value, size);
    if (tcpSocket->isOpen()) {
        while (skt_use)
            QApplication::processEvents();  // TODO only need in rvss, change to mutex
        skt_use = 1;
        uint32_t payload[2];
        payload[0] = htonl(offset);
        payload[1] = htonl(value);
        QByteArray dp = QByteArray(static_cast<char*>((void*)payload), 8);
        if (send_cmd(VB_PWRITE, 8, &dp) < 0) {
            skt_use = 0;
            return;
        }

        cmd_header_t cmd_header;

        if (recv_cmd(cmd_header) < 0) {
            skt_use = 0;
            return;
        }

        if (cmd_header.msg_type == VB_PWRITE) {
            dprintf("write ok\n");
        } else {
            skt_use = 0;
            printf("write error !!!!!!!!!!!!!!!!!!!!!!\n");
            tcpSocket->close();
        }
        skt_use = 0;
    }
}

void IOExternalBus::connectButtonTriggered() {
    if (m_ui->connectButton->text().contains("Connect")) {
        tcpSocket->abort();
        if (tcpSocket->connectToHost(m_ui->address->text(), m_ui->port->value())) {
            if (send_cmd(VB_PINFO) < 0) {
                disconnectOnError();
                return;
            }
            cmd_header_t cmd_header;

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
                dprintf("%s\n", buff.data());

                QJsonDocument desc = QJsonDocument::fromJson(buff.data());
                const unsigned int addrw = desc.object().value(QString("address width")).toInt();
                QJsonObject osymbols = desc.object().value(QString("symbols")).toObject();

                m_regDescs.clear();
                for (QJsonObject::iterator i = osymbols.begin(); i != osymbols.end(); i++) {
                    m_regDescs.push_back(
                        RegDesc{i.key(), RegDesc::RW::RW, addrw * 8, static_cast<AInt>(i.value().toInt()), true});
                }

                ByteSize = addrw * osymbols.count();

                m_ui->connectButton->setText("Disconnect");
                m_ui->status->setText("Connected");
                m_ui->server->setText(desc.object().value(QString("name")).toString());

                emit regMapChanged();
                emit sizeChanged();

                // delete[] buff;
            }
        } else {
            disconnectOnError();
        }
    } else {  // disconnect
        m_ui->connectButton->setText("Connect");
        m_ui->status->setText("Disconnected");
        m_ui->server->setText("-");
        tcpSocket->abort();

        m_regDescs.clear();

        emit regMapChanged();
        emit sizeChanged();
    }
}

int32_t IOExternalBus::send_cmd(const uint32_t cmd, const uint32_t payload_size, const QByteArray* payload) {
    int32_t ret;
    cmd_header_t cmd_header;

    cmd_header.msg_type = htonl(cmd);
    cmd_header.payload_size = htonl(payload_size);

    QByteArray dp = QByteArray(static_cast<const char*>((void*)&cmd_header), sizeof(cmd_header_t));

    if ((ret = tcpSocket->write(dp, sizeof(cmd_header))) < 0) {
        disconnectOnError();
        return -1;
    }

    if (payload_size) {
        int retp = 0;

        if ((retp = tcpSocket->write(*payload, payload_size)) < 0) {
            disconnectOnError();
            return -1;
        }
        ret += retp;
    }
    return ret;
}

int32_t IOExternalBus::recv_cmd(cmd_header_t& cmd_header) {
    QByteArray dp = QByteArray(static_cast<char*>((void*)&cmd_header), sizeof(cmd_header_t));
    int ret = tcpSocket->read(dp, sizeof(cmd_header_t));
    if (ret < 0) {
        disconnectOnError();
        return -1;
    }

    cmd_header_t* hr = (cmd_header_t*)dp.data();

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

void IOExternalBus::disconnectOnError(void) {
    QMessageBox::information(nullptr, tr("Ripes VBus"), tcpSocket->getLastErrorStr());
    tcpSocket->close();

    m_ui->connectButton->setText("Connect");
    m_ui->status->setText("Disconnected");
    m_ui->server->setText("-");
    tcpSocket->abort();

    m_regDescs.clear();

    emit regMapChanged();
    emit sizeChanged();
}

}  // namespace Ripes
