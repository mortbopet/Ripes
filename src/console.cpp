#include "console.h"

#include "ripessettings.h"

#include <QScrollBar>

namespace Ripes {

Console::Console(QWidget* parent) : QPlainTextEdit(parent) {
    QFont font = QFont("Inconsolata", 12);
    setFont(font);
    document()->setMaximumBlockCount(100);

    auto paletteChangeFunctor = [=] {
        QPalette p = palette();
        p.setColor(QPalette::Base, RipesSettings::value(RIPES_SETTING_CONSOLEBG).value<QColor>());
        p.setColor(QPalette::Text, RipesSettings::value(RIPES_SETTING_CONSOLEFONT).value<QColor>());
        setPalette(p);
    };

    connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEBG), &SettingObserver::modified, paletteChangeFunctor);
    connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEFONT), &SettingObserver::modified, paletteChangeFunctor);
    paletteChangeFunctor();

    connect(RipesSettings::getObserver(RIPES_SETTING_CONSOLEECHO), &SettingObserver::modified,
            [=](const QVariant& value) { m_localEchoEnabled = value.toBool(); });
}

void Console::putData(const QByteArray& data) {
    insertPlainText(data);

    QScrollBar* bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
        case Qt::Key_Backspace:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            break;
        default:
            if (m_localEchoEnabled)
                QPlainTextEdit::keyPressEvent(e);
            emit sendData(e->text().toLocal8Bit());
    }
}

}  // namespace Ripes
