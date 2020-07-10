#include "console.h"

#include "ripessettings.h"

#include <QScrollBar>

namespace Ripes {

Console::Console(QWidget* parent) : QPlainTextEdit(parent) {
    if (RipesSettings::value(RIPES_SETTING_CONSOLEFONT).isNull()) {
        m_font = QFont("Inconsolata", 12);
    } else {
        m_font = RipesSettings::value(RIPES_SETTING_CONSOLEFONT).value<QFont>();
    }
    setFont(m_font);

    document()->setMaximumBlockCount(100);

    auto paletteChangeFunctor = [=] {
        QPalette p = palette();
        p.setColor(QPalette::Base, RipesSettings::value(RIPES_SETTING_CONSOLEBG).value<QColor>());
        p.setColor(QPalette::Text, RipesSettings::value(RIPES_SETTING_CONSOLEFONTCOLOR).value<QColor>());
        setPalette(p);
    };

    connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEBG), &SettingObserver::modified, paletteChangeFunctor);
    connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEFONTCOLOR), &SettingObserver::modified,
            paletteChangeFunctor);
    paletteChangeFunctor();

    connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEECHO), &SettingObserver::modified,
            [=](const QVariant& value) { m_localEchoEnabled = value.toBool(); });
    connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEFONT), &SettingObserver::modified,
            [=](const QVariant& value) {
                m_font = value.value<QFont>();
                setFont(m_font);
            });

    m_localEchoEnabled = RipesSettings::value(RIPES_SETTING_CONSOLEECHO).toBool();
}

void Console::putData(const QByteArray& data) {
    // Text can always only be inserted at the end of the console
    auto cursorAtEnd = QTextCursor(document());
    cursorAtEnd.movePosition(QTextCursor::End);
    setTextCursor(cursorAtEnd);
    insertPlainText(data);

    QScrollBar* bar = verticalScrollBar();
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

void Console::keyPressEvent(QKeyEvent* e) {
    bool backspacedBuffer = false;
    switch (e->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            QPlainTextEdit::keyPressEvent(e);
            break;
        default:
            if (!e->text().isEmpty()) {
                const QString text = e->text();
                // Buffer managing
                if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
                    // Return is interpreted as \r\n instead of the default \r
                    m_buffer += "\r\n";

                    // Flush buffer to output
                    emit sendData(m_buffer.toLocal8Bit());
                    m_buffer.clear();
                } else if (e->key() == Qt::Key_Backspace) {
                    if (!m_buffer.isEmpty()) {
                        m_buffer.chop(1);
                        backspacedBuffer = true;
                    }
                } else {
                    m_buffer += text;
                }

                // Console echoing
                if (m_localEchoEnabled) {
                    if (e->key() == Qt::Key_Backspace) {
                        if (backspacedBuffer) {
                            backspace();
                        }
                    } else {
                        putData(text.toUtf8());
                    }
                }
            }
    }
}

}  // namespace Ripes
