#pragma once

#include <QFont>
#include <QPlainTextEdit>

namespace Ripes {

class Console : public QPlainTextEdit {
    Q_OBJECT

signals:
    void sendData(const QByteArray& data);

public:
    Console(QWidget* parent = nullptr);
    void putData(const QByteArray& data);
    void clearConsole();

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    void backspace();

    bool m_localEchoEnabled = false;
    QFont m_font;
    QString m_buffer;
};

}  // namespace Ripes
