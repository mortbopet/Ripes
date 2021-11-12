#ifndef XTCPSOCKET_H
#define XTCPSOCKET_H

class XTcpSocket {
public:
    XTcpSocket();
    ~XTcpSocket();
    void close();
    void abort();
    int write(const char* buff, const int size);
    int read(char* buff, int size);
    int isOpen(void) { return (sockfd >= 0); };
    int connectToHost(const char* host, int port);
    int socketDescriptor(void) { return sockfd; };

    void setSocketDescriptor(int skt) { sockfd = skt; };
    int serverStart(int port);
    int serverAccept(void);
    void serverClose(void);

    static int instances;

private:
    int sockfd = -1;
    int listenfd = -1;
};

#endif /* XTCPSOCKET_H */