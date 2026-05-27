#include "iokeyboard.h"
#include "ioregistry.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMutexLocker>
#include <QPushButton>
#include <QVBoxLayout>

namespace Ripes {

IOKeyboard::IOKeyboard(QWidget *parent) : IOBase(IOType::KEYBOARD, parent) {
  m_parameters[BUFSIZE] = IOParam(BUFSIZE, "Buffer size", 16, true, 1, 256);

  setFocusPolicy(Qt::StrongFocus);
  buildLayout();
  refreshRegMap();
}

QString IOKeyboard::description() const {
  QStringList desc;
  desc << "Memory-mapped keyboard with configurable FIFO buffer.";
  desc << "KEY_DATA (offset 0x00, R): dequeue next ASCII code (0 if empty).";
  desc << "KEY_STATUS (offset 0x04, R/W): read returns buffer count, "
          "write non-zero to clear.";
  return desc.join('\n');
}

void IOKeyboard::buildLayout() {
  auto *root = new QVBoxLayout(this);
  root->setSpacing(4);
  root->setContentsMargins(6, 6, 6, 6);

  auto addKeyButton = [&](QHBoxLayout *row, char ch, int w = 32) {
    auto *btn = new QPushButton(QString(QChar(ch)), this);
    btn->setFixedSize(w, 32);
    btn->setFocusPolicy(Qt::NoFocus);
    const uint8_t ascii = static_cast<uint8_t>(ch);
    connect(btn, &QPushButton::clicked, this,
            [this, ascii]() { enqueueKey(ascii); });
    row->addWidget(btn);
  };

  auto *numRow = new QHBoxLayout();
  numRow->setAlignment(Qt::AlignCenter);
  for (int i = 1; i <= 10; ++i)
    addKeyButton(numRow, '0' + (i % 10));
  root->addLayout(numRow);

  const char *rows[] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
  for (const char *row : rows) {
    auto *rowLayout = new QHBoxLayout();
    rowLayout->setAlignment(Qt::AlignCenter);
    for (int i = 0; row[i]; ++i)
      addKeyButton(rowLayout, row[i]);
    root->addLayout(rowLayout);
  }

  auto *spaceRow = new QHBoxLayout();
  spaceRow->setAlignment(Qt::AlignCenter);
  addKeyButton(spaceRow, ' ', 200);
  root->addLayout(spaceRow);

  m_statusLabel = new QLabel(this);
  root->addWidget(m_statusLabel);
  refreshStatusLabel();
}

void IOKeyboard::refreshRegMap() {
  const unsigned bufSize = m_parameters.at(BUFSIZE).value.toUInt();

  m_regDescs.clear();
  m_regDescs.push_back(RegDesc{"KEY_DATA", RegDesc::RW::R, 8, 0, true});
  m_regDescs.push_back(RegDesc{"KEY_STATUS", RegDesc::RW::RW, 32, 4, true});

  m_extraSymbols.clear();
  m_extraSymbols.push_back(IOSymbol{"BUF_SIZE", bufSize});

  emit regMapChanged();
}

void IOKeyboard::enqueueKey(uint8_t ascii) {
  {
    QMutexLocker lock(&m_bufMutex);
    const unsigned maxSize = m_parameters.at(BUFSIZE).value.toUInt();
    if (static_cast<unsigned>(m_keyBuffer.size()) < maxSize)
      m_keyBuffer.enqueue(ascii);
    m_lastKey = ascii;
  }
  refreshStatusLabel();
}

void IOKeyboard::refreshStatusLabel() {
  if (!m_statusLabel)
    return;

  int count;
  uint8_t ch;

  {
    QMutexLocker lock(&m_bufMutex);
    count = m_keyBuffer.size();
    ch = m_lastKey;
  }

  const unsigned bufSize = m_parameters.at(BUFSIZE).value.toUInt();
  const QString charStr =
      (ch >= 0x20 && ch < 0x7F) ? QString(QChar(ch)) : QString("--");

  m_statusLabel->setText(QString("Last: %1 (0x%2) | Buffer: %3/%4")
                             .arg(charStr)
                             .arg(ch, 2, 16, QChar('0'))
                             .arg(count)
                             .arg(bufSize));
}

void IOKeyboard::keyPressEvent(QKeyEvent *event) {
  if (event->isAutoRepeat()) {
    event->ignore();
    return;
  }

  const int key = event->key();
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
  } else {
    event->ignore();
  }
}

void IOKeyboard::parameterChanged(unsigned) {
  refreshRegMap();
  refreshStatusLabel();
}

VInt IOKeyboard::ioRead(AInt offset, unsigned) {
  if (offset == 0) {
    uint8_t val;

    {
      QMutexLocker lock(&m_bufMutex);
      val = m_keyBuffer.isEmpty() ? 0 : m_keyBuffer.dequeue();
    }

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
    {
      QMutexLocker lock(&m_bufMutex);
      m_keyBuffer.clear();
    }

    QMetaObject::invokeMethod(
        this, [this]() { refreshStatusLabel(); }, Qt::QueuedConnection);
  }
}

void IOKeyboard::reset() {
  {
    QMutexLocker lock(&m_bufMutex);
    m_keyBuffer.clear();
    m_lastKey = 0;
  }
  refreshStatusLabel();
}

} // namespace Ripes