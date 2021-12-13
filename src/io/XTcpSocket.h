#ifndef XTCPSOCKET_H
#define XTCPSOCKET_H

#include <QByteArray>
#include <QString>

class XTcpSocket {
public:
    XTcpSocket();
    ~XTcpSocket();
    void close();
    void abort();
    int write(const QByteArray& buff, const size_t size);
    int read(QByteArray& buff, const size_t size);
    int isOpen(void) { return (sockfd >= 0); };
    int connectToHost(const QString& host, int port);
    int socketDescriptor(void) { return sockfd; };

    void setSocketDescriptor(int skt) { sockfd = skt; };
    int serverStart(int port);
    int serverAccept(void);
    void serverClose(void);
    const QString getLastErrorStr(void);
    static std::atomic<int> instances;

private:
    void FormatLastErrorStr(const QString& func);

    QString lastError;
    int sockfd = -1;
    int listenfd = -1;
};

#endif /* XTCPSOCKET_H */