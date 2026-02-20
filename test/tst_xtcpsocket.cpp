#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QtTest/QTest>

#include "io/XTcpSocket.h"

class tst_xtcpsocket : public QObject {
  Q_OBJECT

private:
  XTcpSocket client;
  XTcpSocket server;

private slots:
  void tst_pinpong();
};

void tst_xtcpsocket::tst_pinpong() {
  const QByteArray in("0123456789", 10);
  QByteArray out(10, 0);

  if (server.serverStart(7890) < 0) {
    QFAIL(server.getLastErrorStr().toStdString().c_str());
  }
  QFuture<int> future = QtConcurrent::run(&XTcpSocket::serverAccept, &server);
  if (client.connectToHost("127.0.0.1", 7890) < 0) {
    QFAIL(client.getLastErrorStr().toStdString().c_str());
  }
  future.waitForFinished();

  out.fill(' ', 10);
  if (server.write(in, 10) < 0) {
    QFAIL(server.getLastErrorStr().toStdString().c_str());
  }
  if (client.read(out, 10) < 0) {
    QFAIL(client.getLastErrorStr().toStdString().c_str());
  }

  QCOMPARE(in, out);

  out.fill(' ', 10);
  if (client.write(in, 10) < 0) {
    QFAIL(client.getLastErrorStr().toStdString().c_str());
  }
  if (server.read(out, 10) < 0) {
    QFAIL(server.getLastErrorStr().toStdString().c_str());
  }

  QCOMPARE(in, out);

  server.serverClose();
  server.close();
  client.close();
}

QTEST_APPLESS_MAIN(tst_xtcpsocket)
#include "tst_xtcpsocket.moc"
