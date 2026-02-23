#include "io7indicator.h"
#include "ioregistry.h"

#include <QPainter>
#include <algorithm>

namespace Ripes {


unsigned IO7Indicator::numDigits() const {
  auto it = m_parameters.find(DIGITS);
  if (it == m_parameters.end())
    return 4;
  unsigned n = it->second.value.toUInt();
  return (n >= 1 && n <= 8) ? n : 4;
}

void IO7Indicator::rebuildRegDescs() {
  const unsigned n = numDigits();
  m_regDescs.clear();
  m_regDescs.reserve(n);
  for (unsigned i = 0; i < n; i++) {
    m_regDescs.push_back(RegDesc{QString("Digit %1").arg(i), RegDesc::RW::RW, 8,
                                 static_cast<AInt>(i * 4), true});
  }
}

IO7Indicator::IO7Indicator(QWidget *parent)
    : IOBase(IOType::SEVEN_SEGMENT, parent) {
  m_parameters[DIGITS] = IOParam(DIGITS, "# Digits", 4, true, 1, 8);
  m_parameters[DIGIT_SIZE] =
      IOParam(DIGIT_SIZE, "Digit size", 64, true, 30, 120);

  m_digitValues.assign(numDigits(), 0);
  rebuildRegDescs();
}

QString IO7Indicator::description() const {
  return QStringLiteral("7-segment indicator.\n"
                        "Each digit is a 4-byte word (offset = index * 4).\n"
                        "Bits 0-6 = segments a-g, bit 7 = decimal point.");
}

unsigned IO7Indicator::byteSize() const { return numDigits() * 4; }

void IO7Indicator::parameterChanged(unsigned /*ID*/) {
  if (m_updating)
    return;
  m_updating = true;

  const unsigned n = numDigits();
  m_digitValues.resize(n, 0);
  rebuildRegDescs();

  updateGeometry();
  update();
  emit regMapChanged();
  emit sizeChanged();

  m_updating = false;
}

VInt IO7Indicator::ioRead(AInt offset, unsigned size) {
  VInt r = 0;
  for (unsigned i = 0; i < size; i++) {
    AInt addr = offset + i;
    unsigned idx = static_cast<unsigned>(addr / 4);
    if (addr % 4 == 0 && idx < m_digitValues.size())
      r |= static_cast<VInt>(m_digitValues[idx]) << (i * 8);
  }
  return r;
}

void IO7Indicator::ioWrite(AInt offset, VInt value, unsigned size) {
  for (unsigned i = 0; i < size; i++) {
    AInt addr = offset + i;
    unsigned idx = static_cast<unsigned>(addr / 4);
    if (addr % 4 == 0 && idx < m_digitValues.size())
      m_digitValues[idx] = static_cast<uint8_t>((value >> (i * 8)) & 0xFF);
  }
  emit scheduleUpdate();
}

void IO7Indicator::reset() {
  std::fill(m_digitValues.begin(), m_digitValues.end(), 0);
  emit scheduleUpdate();
}

void IO7Indicator::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.fillRect(rect(), QColor(20, 20, 20));

  const int n = static_cast<int>(m_digitValues.size());
  if (n == 0)
    return;

  auto it = m_parameters.find(DIGIT_SIZE);
  const int h = (it != m_parameters.end()) ? it->second.value.toInt() : 64;
  const int w = h * 2 / 3;
  const int gap = 6;
  const int totalW = n * w + (n - 1) * gap;
  const int x0 = (width() - totalW) / 2;
  const int y0 = (height() - h) / 2;

  for (int i = 0; i < n; i++)
    drawDigit(p, x0 + i * (w + gap), y0, w, h, m_digitValues[i]);
}

void IO7Indicator::drawDigit(QPainter &p, int x, int y, int w, int h,
                             uint8_t seg) {
  const QColor on(255, 30, 30), off(50, 10, 10);
  const qreal t = qMin(w, h) * 0.12;

  const qreal l = x + t, r = x + w - t;
  const qreal top = y + t, mid = y + h / 2.0, bot = y + h - t;

  const QLineF lines[7] = {
      {l, top, r, top}, // a — top
      {r, top, r, mid}, // b — right top
      {r, mid, r, bot}, // c — right bot
      {l, bot, r, bot}, // d — bot
      {l, mid, l, bot}, // e — left bot
      {l, top, l, mid}, // f — left top
      {l, mid, r, mid}, // g — mid
  };

  for (int i = 0; i < 7; i++) {
    p.setPen(QPen((seg >> i) & 1 ? on : off, t, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(lines[i]);
  }
}

} // namespace Ripes