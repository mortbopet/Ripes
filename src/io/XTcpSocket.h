

class XTcpSocket {
public:
    XTcpSocket() { sockfd = -1; };
    void close();
    void abort();
    int write(const char* buff, const int size) ;
    int read(char* buff, int size);
    int isOpen(void) { return (sockfd >= 0); };
    int connectToHost(const char* host, int port);  
    int socketDescriptor(void) { return sockfd; };
private:
    int sockfd;
};
