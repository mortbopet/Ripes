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

void XTcpSocket::FormatLastErrorStr(const QString &func) {
#ifdef _MSC_VER
  int err;
  TCHAR msgbuf[256]; // for a message up to 255 bytes.
  msgbuf[0] = '\0';  // Microsoft doesn't guarantee this on man page.
  err = WSAGetLastError();
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,         // flags
                NULL,                                      // lpsource
                err,                                       // message id
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // languageid
                msgbuf,                                    // output buffer
                sizeof(msgbuf), // size of msgbuf, bytes
                NULL);          // va_list of arguments

  if (!*msgbuf)
    lastError = func + " error number : " +
                QString(err); // provide error # if no string available
  else
    lastError = func + " error : " + msgbuf;

#else
  lastError = func + " error : " + strerror(errno);
#endif
}

const QString XTcpSocket::getLastErrorStr(void) { return lastError; }

void XTcpSocket::close() {
  // shutdown (sockfd,SHUT_RDWR );
#ifdef _MSC_VER
  closesocket(sockfd);
#else
  ::close(sockfd);
#endif
  sockfd = -1;
}

void XTcpSocket::abort() { close(); }

int XTcpSocket::write(const QByteArray &buff, const size_t size) {
  int ret =
      send(sockfd, static_cast<const char *>(buff.data()), size, MSG_NOSIGNAL);
  if (ret != (int)size) {
    FormatLastErrorStr("write");
    return -1;
  }
  return ret;
}

int XTcpSocket::read(QByteArray &buff, const size_t size) {
  int ret = recv(sockfd, static_cast<char *>(buff.data()), size, MSG_WAITALL);
  if (ret != (int)size) {
    FormatLastErrorStr("read");
    return -1;
  }
  return ret;
}

int XTcpSocket::connectToHost(const QString &host, int port) {
  struct sockaddr_in serv;

  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    FormatLastErrorStr("socket");
    return 0;
  }

  memset(&serv, 0, sizeof(serv));
  serv.sin_family = AF_INET;
  serv.sin_addr.s_addr = inet_addr(host.toStdString().c_str());
  serv.sin_port = htons(port);

  if (connect(sockfd, reinterpret_cast<struct sockaddr *>(&serv),
              sizeof(serv)) < 0) {
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
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char *>(&reuse), sizeof(reuse)) < 0) {
    FormatLastErrorStr("setsockopt(SO_REUSEADDR)");
  }

  memset(&serv, 0, sizeof(serv));
  serv.sin_family = AF_INET;
  serv.sin_addr.s_addr = htonl(INADDR_ANY);
  serv.sin_port = htons(port);

  if (bind(listenfd, reinterpret_cast<struct sockaddr *>(&serv), sizeof(serv)) <
      0) {
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
  int clilen = sizeof(cli); // accept third argument is int on MSVC
#else
  unsigned int clilen =
      sizeof(cli); // accpet third argument is unsigned int in gcc
#endif
  sockfd = accept(listenfd, reinterpret_cast<struct sockaddr *>(&cli), &clilen);
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

std::atomic<int> XTcpSocket::instances = 0;

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
