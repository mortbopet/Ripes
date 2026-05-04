#include "iokeyboard.h"
#include "ioregistry.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMutexLocker>
#include <QPushButton>
#include <QVBoxLayout>

namespace Ripes {

    IOKeyboard::IOKeyboard(QWidget* parent) : IOBase(IOType::KEYBOARD, parent) {
        m_parameters[BUFSIZE] = IOParam(BUFSIZE, "Buffer size", 16, true, 1, 256);

        setFocusPolicy(Qt::StrongFocus);
        updateLayout();
    }

    QString IOKeyboard::description() const {
        QStringList desc;
        desc << "Memory-mapped keyboard with configurable FIFO buffer.";
        desc << "KEY_DATA (offset 0x00, R): dequeue next ASCII code (0 if empty).";
        desc << "KEY_STATUS (offset 0x04, R/W): read returns buffer count, "
            "write non-zero to clear.";
        return desc.join('\n');
    }

    void IOKeyboard::updateLayout() {
        m_keys.clear();
        m_statusLabel = nullptr;

        if (layout()) {
            QWidget dummy;
            dummy.setLayout(layout());
        }

        auto* root = new QVBoxLayout(this);
        root->setSpacing(4);
        root->setContentsMargins(6, 6, 6, 6);

        auto* numRow = new QHBoxLayout();
        numRow->setAlignment(Qt::AlignCenter);
        for (int i = 1; i <= 10; ++i) {
            char ch = '0' + (i % 10);
            auto* btn = new QPushButton(QString(QChar(ch)), this);
            btn->setFixedSize(32, 32);
            btn->setFocusPolicy(Qt::NoFocus);
            uint8_t ascii = static_cast<uint8_t>(ch);
            connect(btn, &QPushButton::clicked, this,
                [this, ascii]() { enqueueKey(ascii); });
            m_keys[ascii] = btn;
            numRow->addWidget(btn);
        }
        root->addLayout(numRow);

        const char* rows[] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };
        for (const char* row : rows) {
            auto* rowLayout = new QHBoxLayout();
            rowLayout->setAlignment(Qt::AlignCenter);
            for (int i = 0; row[i]; ++i) {
                char ch = row[i];
                auto* btn = new QPushButton(QString(QChar(ch)), this);
                btn->setFixedSize(32, 32);
                btn->setFocusPolicy(Qt::NoFocus);
                uint8_t ascii = static_cast<uint8_t>(ch);
                connect(btn, &QPushButton::clicked, this,
                    [this, ascii]() { enqueueKey(ascii); });
                m_keys[ascii] = btn;
                rowLayout->addWidget(btn);
            }
            root->addLayout(rowLayout);
        }

        auto* spaceRow = new QHBoxLayout();
        spaceRow->setAlignment(Qt::AlignCenter);
        auto* spaceBtn = new QPushButton("SPACE", this);
        spaceBtn->setFixedSize(200, 32);
        spaceBtn->setFocusPolicy(Qt::NoFocus);
        connect(spaceBtn, &QPushButton::clicked, this,
            [this]() { enqueueKey(static_cast<uint8_t>(' ')); });
        m_keys[static_cast<uint8_t>(' ')] = spaceBtn;
        spaceRow->addWidget(spaceBtn);
        root->addLayout(spaceRow);

        const unsigned bufSize = m_parameters.at(BUFSIZE).value.toUInt();
        m_statusLabel =
            new QLabel(QString("Last: -- | Buffer: 0/%1").arg(bufSize), this);
        root->addWidget(m_statusLabel);

        m_regDescs.clear();
        m_regDescs.push_back(RegDesc{ "KEY_DATA", RegDesc::RW::R, 8, 0, true });
        m_regDescs.push_back(RegDesc{ "KEY_STATUS", RegDesc::RW::RW, 32, 4, true });

        m_extraSymbols.clear();
        m_extraSymbols.push_back(IOSymbol{ "BUF_SIZE", bufSize });

        updateGeometry();
        emit regMapChanged();
    }

    void IOKeyboard::enqueueKey(uint8_t ascii) {
        {
            QMutexLocker lock(&m_bufMutex);
            unsigned maxSize = m_parameters.at(BUFSIZE).value.toUInt();
            if (static_cast<unsigned>(m_keyBuffer.size()) < maxSize)
                m_keyBuffer.enqueue(ascii);
            m_lastKey = ascii;
        }
        refreshStatusLabel();
    }

    void IOKeyboard::refreshStatusLabel() {
        QMutexLocker lock(&m_bufMutex);
        int count = m_keyBuffer.size();
        uint8_t ch = m_lastKey;
        lock.unlock();

        if (!m_statusLabel)
            return;

        const unsigned bufSize = m_parameters.at(BUFSIZE).value.toUInt();
        QString charStr =
            (ch >= 0x20 && ch < 0x7F) ? QString(QChar(ch)) : QString("--");
        m_statusLabel->setText(
            QString("Last: %1 (0x%2) | Buffer: %3/%4")
            .arg(charStr)
            .arg(ch, 2, 16, QChar('0'))
            .arg(count)
            .arg(bufSize));
    }

    void IOKeyboard::keyPressEvent(QKeyEvent* event) {
        if (event->isAutoRepeat()) {
            event->ignore();
            return;
        }

        int key = event->key();
        uint8_t ascii = 0;

        if (key >= Qt::Key_A && key <= Qt::Key_Z)
            ascii = static_cast<uint8_t>('A' + (key - Qt::Key_A));
        else if (key >= Qt::Key_0 && key <= Qt::Key_9)
            ascii = static_cast<uint8_t>('0' + (key - Qt::Key_0));
        else if (key == Qt::Key_Space)
            ascii = static_cast<uint8_t>(' ');

        if (ascii != 0) {
            enqueueKey(ascii);
            event->accept();
        }
        else {
            event->ignore();
        }
    }

    void IOKeyboard::parameterChanged(unsigned) {
        updateLayout();
    }

    VInt IOKeyboard::ioRead(AInt offset, unsigned) {
        if (offset == 0) {
            QMutexLocker lock(&m_bufMutex);
            uint8_t val = m_keyBuffer.isEmpty() ? 0 : m_keyBuffer.dequeue();
            lock.unlock();
            QMetaObject::invokeMethod(
                this, [this]() { refreshStatusLabel(); }, Qt::QueuedConnection);
            return val;
        }
        if (offset == 4) {
            QMutexLocker lock(&m_bufMutex);
            return static_cast<VInt>(m_keyBuffer.size());
        }
        return 0;
    }

    void IOKeyboard::ioWrite(AInt offset, VInt value, unsigned) {
        if (offset == 4 && value != 0) {
            QMutexLocker lock(&m_bufMutex);
            m_keyBuffer.clear();
            lock.unlock();
            QMetaObject::invokeMethod(
                this, [this]() { refreshStatusLabel(); }, Qt::QueuedConnection);
        }
    }

    void IOKeyboard::reset() {
        QMutexLocker lock(&m_bufMutex);
        m_keyBuffer.clear();
        m_lastKey = 0;
        lock.unlock();
        refreshStatusLabel();
    }

}