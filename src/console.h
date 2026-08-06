#pragma once

#include <QTextBlock>

#include <QFont>
#include <QPlainTextEdit>

namespace Ripes {

class Console : public QPlainTextEdit {
  Q_OBJECT

signals:
  void sendData(const QByteArray &data);

public:
  Console(QWidget *parent = nullptr);
  void putData(const QByteArray &data);
  void clearConsole();

protected:
  void keyPressEvent(QKeyEvent *e) override;
  void resizeEvent(QResizeEvent *) override;
  void moveEvent(QMoveEvent *) override;

private:
  void backspace();

  bool m_localEchoEnabled = false;
  QFont m_font;
  QString m_buffer;

  // window size & state.
  QVariant m_savedSize;
  Qt::WindowStates m_savedState;
};

} // namespace Ripes
