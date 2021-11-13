#ifndef XTCPSOCKET_H
#define XTCPSOCKET_H

#include <QByteArray>
#include <QString>

class XTcpSocket {  //: public QObject {
    // Q_OBJECT
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
    const char* getLastErrorStr(void);
    static int instances;

    // signals:
    //   void onError(const QString& msg);

private:
    void FormatLastErrorStr(const char* func);

    char lastError[256];
    int sockfd = -1;
    int listenfd = -1;
};

#endif /* XTCPSOCKET_H */