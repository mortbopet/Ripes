#include "console.h"

#include "fonts.h"
#include "ripessettings.h"

#include <QScrollBar>

#include <QClipboard>
#include <QEvent>
#include <QGuiApplication>

namespace Ripes {

Console::Console(QWidget *parent) : QPlainTextEdit(parent) {
  if (RipesSettings::value(RIPES_SETTING_CONSOLEFONT).isNull()) {
    m_font = QFont(Fonts::monospace, 12);
  } else {
    m_font = RipesSettings::value(RIPES_SETTING_CONSOLEFONT).value<QFont>();
  }
  setFont(m_font);

  document()->setMaximumBlockCount(100);

  connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEBG),
          &SettingObserver::modified, this, [this] { applyThemeColors(); });
  connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEFONTCOLOR),
          &SettingObserver::modified, this, [this] { applyThemeColors(); });
  connect(ThemeManager::get(), &ThemeManager::themeChanged, this,
          [this] { applyThemeColors(); });
  applyThemeColors();

  connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEECHO),
          &SettingObserver::modified, this, [this](const QVariant &value) {
            m_localEchoEnabled = value.toBool();
          });
  connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEFONT),
          &SettingObserver::modified, this, [this](const QVariant &value) {
            m_font = value.value<QFont>();
            setFont(m_font);
          });

  m_localEchoEnabled = RipesSettings::value(RIPES_SETTING_CONSOLEECHO).toBool();
}

void Console::applyThemeColors() {
  const auto bg = RipesSettings::value(RIPES_SETTING_CONSOLEBG).value<QColor>();
  const auto fg =
      RipesSettings::value(RIPES_SETTING_CONSOLEFONTCOLOR).value<QColor>();
  // With no custom colors, reset to an empty palette so the console follows the
  // application (light/dark) theme. Otherwise, apply the user's colors on top
  // of the current theme palette.
  if (!bg.isValid() && !fg.isValid()) {
    setPalette(QPalette());
    return;
  }
  QPalette p = palette();
  if (bg.isValid())
    p.setColor(QPalette::Base, bg);
  if (fg.isValid())
    p.setColor(QPalette::Text, fg);
  setPalette(p);
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
  QClipboard *clipboard = QGuiApplication::clipboard();

  if (e->modifiers() & Qt::ControlModifier) {
    switch (e->key()) {
    case Qt::Key_C:
      if (textCursor().hasSelection()) {
        clipboard->setText(textCursor().selectedText());
      }
      return;

    case Qt::Key_V:
      QString clipboardText = clipboard->text();
      if (!clipboardText.isEmpty()) {
        m_buffer += clipboardText;
        if (m_localEchoEnabled)
          putData(clipboardText.toUtf8());
      }
      return;
    }
  }

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
