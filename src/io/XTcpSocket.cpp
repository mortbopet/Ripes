#include "XTcpSocket.h"

// platform dependent system headers
#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#define SHUT_RDWR SD_BOTH
#define MSG_NOSIGNAL 0
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#endif

// platform independent system headers
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void XTcpSocket::FormatLastErrorStr(const char* func) {
#ifdef _MSC_VER
    int err;

    char msgbuf[256];  // for a message up to 255 bytes.

    msgbuf[0] = '\0';  // Microsoft doesn't guarantee this on man page.

    err = WSAGetLastError();

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,  // flags
                  NULL,                                                        // lpsource
                  err,                                                         // message id
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),                   // languageid
                  msgbuf,                                                      // output buffer
                  sizeof(msgbuf),                                              // size of msgbuf, bytes
                  NULL);                                                       // va_list of arguments

    if (!*msgbuf)
        snprintf(lastError, 255, "%s error : %d ", err);  // provide error # if no string available
    else
        snprintf(lastError, 255, "%s error : %s ", func, msgbuf);

#else
    snprintf(lastError, 255, "%s error : %s ", func, strerror(errno));
#endif
}

const char* XTcpSocket::getLastErrorStr(void) {
    return lastError;
}

void XTcpSocket::close() {
#ifdef _MSC_VER
    closesocket(sockfd);
#else
    ::close(sockfd);
#endif
    sockfd = -1;
}

void XTcpSocket::abort() {
#ifdef _MSC_VER
    closesocket(sockfd);
#else
    ::close(sockfd);
#endif
    sockfd = -1;
}

int XTcpSocket::write(const char* buff, const int size) {
    int ret = send(sockfd, buff, size, MSG_NOSIGNAL);
    if (ret != size) {
        FormatLastErrorStr("write");
        return -1;
    }
    return ret;
}

int XTcpSocket::read(char* buff, int size) {
    int ret = recv(sockfd, buff, size, MSG_WAITALL);
    if (ret != size) {
        FormatLastErrorStr("read");
        return -1;
    }
    return ret;
}

int XTcpSocket::connectToHost(const char* host, int port) {
    struct sockaddr_in serv;

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        FormatLastErrorStr("socket");
        return 0;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(host);
    serv.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        FormatLastErrorStr("connect");
        close();
        return 0;
    }

    return 1;
}

int XTcpSocket::serverStart(int port) {
    struct sockaddr_in serv;

    if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        FormatLastErrorStr("socket");
        return -1;
    }

    int reuse = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        FormatLastErrorStr("setsockopt(SO_REUSEADDR)");
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        FormatLastErrorStr("bind");
        serverClose();
        return -1;
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        FormatLastErrorStr("listen");
        serverClose();
        return -1;
    }

    return listenfd;
}

int XTcpSocket::serverAccept(void) {
    struct sockaddr_in cli;
#ifdef _MSC_VER
    int clilen = sizeof(cli);
#else
    unsigned int clilen = sizeof(cli);
#endif
    sockfd = accept(listenfd, (struct sockaddr*)&cli, &clilen);
    if (sockfd < 0) {
        FormatLastErrorStr("accept");
        sockfd = -1;
    }
    return sockfd;
}

void XTcpSocket::serverClose(void) {
#ifdef _MSC_VER
    closesocket(listenfd);
#else
    ::close(listenfd);
#endif
    listenfd = -1;
}

int XTcpSocket::instances = 0;

XTcpSocket::XTcpSocket() {
#ifdef _MSC_VER
    if (!XTcpSocket::instances) {
        WORD wVersionRequested = 2;
        WSADATA wsaData;

        WSAStartup(wVersionRequested, &wsaData);
        if (wsaData.wVersion != wVersionRequested) {
            fprintf(stderr, "\n Wrong version\n");
            return;
        }
    }
#endif
    ++XTcpSocket::instances;
}

XTcpSocket::~XTcpSocket() {
    --XTcpSocket::instances;
#ifdef _MSC_VER
    if (!XTcpSocket::instances) {
        WSACleanup();
    }
#endif
}
