#include "console.h"

#include "fonts.h"
#include "ripessettings.h"

#include <QScrollBar>

namespace Ripes {

Console::Console(QWidget *parent) : QPlainTextEdit(parent) {
  if (RipesSettings::value(RIPES_SETTING_CONSOLEFONT).isNull()) {
    m_font = QFont(Fonts::monospace, 12);
  } else {
    m_font = RipesSettings::value(RIPES_SETTING_CONSOLEFONT).value<QFont>();
  }
  setFont(m_font);

  document()->setMaximumBlockCount(100);

  auto paletteChangeFunctor = [=] {
    QPalette p = palette();
    p.setColor(QPalette::Base,
               RipesSettings::value(RIPES_SETTING_CONSOLEBG).value<QColor>());
    p.setColor(
        QPalette::Text,
        RipesSettings::value(RIPES_SETTING_CONSOLEFONTCOLOR).value<QColor>());
    setPalette(p);
  };

  connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEBG),
          &SettingObserver::modified, paletteChangeFunctor);
  connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEFONTCOLOR),
          &SettingObserver::modified, paletteChangeFunctor);
  paletteChangeFunctor();

  connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEECHO),
          &SettingObserver::modified, this,
          [=](const QVariant &value) { m_localEchoEnabled = value.toBool(); });
  connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEFONT),
          &SettingObserver::modified, this, [=](const QVariant &value) {
            m_font = value.value<QFont>();
            setFont(m_font);
          });

  m_localEchoEnabled = RipesSettings::value(RIPES_SETTING_CONSOLEECHO).toBool();
}

void Console::putData(const QByteArray &bytes) {
  // Text can always only be inserted at the end of the console
  auto cursorAtEnd = QTextCursor(document());
  cursorAtEnd.movePosition(QTextCursor::End);
  setTextCursor(cursorAtEnd);
  insertPlainText(bytes);

  QScrollBar *bar = verticalScrollBar();
  bar->setValue(bar->maximum());
}

void Console::clearConsole() {
  clear();
  m_buffer.clear();
}

void Console::backspace() {
  // Deletes the last character in the console
  auto cursorAtEnd = QTextCursor(document());
  cursorAtEnd.movePosition(QTextCursor::End);
  setTextCursor(cursorAtEnd);
  textCursor().deletePreviousChar();
}

void Console::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Left:
  case Qt::Key_Right:
  case Qt::Key_Up:
  case Qt::Key_Down:
    QPlainTextEdit::keyPressEvent(e);
    break;

  case Qt::Key_Return:
  case Qt::Key_Enter:
    // Return is interpreted as \n instead of the default \r (\n)
    m_buffer += "\n";

    // Flush buffer to output
    emit sendData(m_buffer.toLocal8Bit());
    m_buffer.clear();

    if (m_localEchoEnabled)
      putData("\r");
    break;

  case Qt::Key_Backspace:
    if (!m_buffer.isEmpty()) {
      // Remove the last character from the buffer
      m_buffer.chop(1);
      if (m_localEchoEnabled)
        backspace();
    }
    break;

  default:
    if (!e->text().isEmpty()) {
      m_buffer += e->text();
      if (m_localEchoEnabled)
        putData(e->text().toUtf8());
    }
  }
}

} // namespace Ripes
