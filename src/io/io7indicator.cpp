#include "io7indicator.h"
#include "ioregistry.h"

#include <QPainter>
#include <QSizePolicy>

namespace Ripes {

namespace {

struct SegColor {
  QColor on;
  QColor off;
};

static const SegColor s_segColors[] = {
    {{255, 30, 30}, {50, 10, 10}},
    {{30, 255, 30}, {10, 50, 10}},
    {{50, 130, 255}, {15, 25, 55}},
    {{255, 220, 30}, {55, 48, 10}},
    {{230, 230, 240}, {45, 45, 50}},
};

static constexpr int s_numColors =
    static_cast<int>(sizeof(s_segColors) / sizeof(s_segColors[0]));
static constexpr qreal s_digitAspect = 0.56;
static constexpr qreal s_gapRatio   = 0.12;

QPolygonF horizontalSegment(const QRectF &rect) {
  const qreal bevel = qMin(rect.height() * 0.8, rect.width() / 3.0);
  const qreal cy = rect.center().y();

  QPolygonF polygon;
  polygon << QPointF(rect.left() + bevel, rect.top())
          << QPointF(rect.right() - bevel, rect.top())
          << QPointF(rect.right(), cy)
          << QPointF(rect.right() - bevel, rect.bottom())
          << QPointF(rect.left() + bevel, rect.bottom())
          << QPointF(rect.left(), cy);
  return polygon;
}

QPolygonF verticalSegment(const QRectF &rect) {
  const qreal bevel = qMin(rect.width() * 0.8, rect.height() / 3.0);
  const qreal cx = rect.center().x();

  QPolygonF polygon;
  polygon << QPointF(rect.left(), rect.top() + bevel)
          << QPointF(cx, rect.top())
          << QPointF(rect.right(), rect.top() + bevel)
          << QPointF(rect.right(), rect.bottom() - bevel)
          << QPointF(cx, rect.bottom())
          << QPointF(rect.left(), rect.bottom() - bevel);
  return polygon;
}

}

IO7Indicator::IO7Indicator(QWidget *parent)
    : IOBase(IOType::SEVEN_SEGMENT, parent) {
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  m_parameters[NUM_DIGITS] =
      IOParam(NUM_DIGITS, "# Digits", DEFAULT_NUM_DIGITS, true,
              MIN_NUM_DIGITS, MAX_NUM_DIGITS);
  m_parameters[COLOR] =
      IOParam(COLOR, "Color", 0, true, 0, s_numColors - 1);

  m_digitValues.assign(DEFAULT_NUM_DIGITS, 0);
  setMinimumSize(minimumSizeHint());
  updateRegDescs();
}

unsigned IO7Indicator::numDigits() const {
  return static_cast<unsigned>(m_parameters.at(NUM_DIGITS).value.toInt());
}

QString IO7Indicator::description() const {
  QStringList desc;
  desc << "7-segment display with " + QString::number(numDigits()) +
              " digits.";
  desc << "Each digit is a 4-byte word (offset = index * 4).";
  desc << "Bits 0-6 = segments a-g, bit 7 = decimal point.";
  return desc.join('\n');
}

unsigned IO7Indicator::byteSize() const {
  return numDigits() * 4;
}

void IO7Indicator::updateRegDescs() {
  const unsigned n = numDigits();
  m_digitValues.resize(n, 0);

  m_regDescs.clear();
  for (unsigned i = 0; i < n; ++i) {
    m_regDescs.push_back(
        RegDesc{QString("Digit %1").arg(i), RegDesc::RW::RW, 8,
                static_cast<AInt>(i * 4), true});
  }

  m_extraSymbols.clear();
  m_extraSymbols.push_back(IOSymbol{"N_DIGITS", n});

  updateGeometry();
  emit regMapChanged();
}

void IO7Indicator::parameterChanged(unsigned ID) {
  if (ID == NUM_DIGITS)
    updateRegDescs();
  setMinimumSize(minimumSizeHint());
  updateGeometry();
  update();
}

VInt IO7Indicator::ioRead(AInt offset, unsigned size) {
  if (size != 4 || (offset % 4) != 0)
    return 0;

  const unsigned idx = static_cast<unsigned>(offset / 4);
  if (idx >= m_digitValues.size())
    return 0;

  return static_cast<VInt>(m_digitValues[idx]);
}

void IO7Indicator::ioWrite(AInt offset, VInt value, unsigned size) {
  if (size != 4 || (offset % 4) != 0)
    return;

  const unsigned idx = static_cast<unsigned>(offset / 4);
  if (idx >= m_digitValues.size())
    return;

  m_digitValues[idx] = static_cast<uint8_t>(value & 0xFF);
  emit scheduleUpdate();
}

void IO7Indicator::reset() {
  std::fill(m_digitValues.begin(), m_digitValues.end(), 0);
  emit scheduleUpdate();
}

QSize IO7Indicator::minimumSizeHint() const {
  static constexpr int minH = 64;
  const int n      = static_cast<int>(numDigits());
  const int digitW = qRound(minH * s_digitAspect);
  const int gap    = qMax(4, qRound(minH * s_gapRatio));
  const int margin = 12;
  return QSize(n * digitW + (n - 1) * gap + margin * 2, minH + margin * 2);
}

void IO7Indicator::drawDigit(QPainter &p, int x, int y, int w, int h,
                             uint8_t seg) {
  if (w <= 0 || h <= 0)
    return;

  const int ci =
      qBound(0, m_parameters.at(COLOR).value.toInt(), s_numColors - 1);
  const QColor &on  = s_segColors[ci].on;
  const QColor &off = s_segColors[ci].off;

  // Classic 7-seg proportions: slimmer segments with tiny inter-seg gap
  const qreal T  = h * 0.075;
  const qreal G  = T * 0.25;                       // gap between segments
  const qreal vH = (h - 3.0 * T - 4.0 * G) / 2.0; // vertical seg length

  // Horizontal columns
  const qreal lx  = x;                  // left vert x
  const qreal rx  = x + w - T;          // right vert x
  const qreal x0  = lx + T + G;         // horiz seg left
  const qreal x1  = rx - G;             // horiz seg right
  const qreal hw  = x1 - x0;            // horiz seg width

  // Row tops (A=0, F/B=1, G=2, E/C=3, D=4)
  const qreal yA = y;
  const qreal yF = yA + T + G;
  const qreal yG = yF + vH + G;
  const qreal yE = yG + T + G;
  const qreal yD = yE + vH + G;

  // Upright digits (no italic shear)
  auto lean = [&](qreal px, qreal py) -> QPointF {
    return {px, py};
  };

  auto hPoly = [&](qreal sx, qreal sy, qreal sw, qreal st) {
    const qreal bev = qMin(st * 0.85, sw / 5.0);
    const qreal cy  = sy + st / 2.0;
    QPolygonF poly;
    poly << lean(sx + bev,      sy)
         << lean(sx + sw - bev, sy)
         << lean(sx + sw,       cy)
         << lean(sx + sw - bev, sy + st)
         << lean(sx + bev,      sy + st)
         << lean(sx,            cy);
    return poly;
  };

  auto vPoly = [&](qreal sx, qreal sy, qreal sw, qreal sh) {
    const qreal bev = qMin(sw * 0.85, sh / 5.0);
    const qreal cx  = sx + sw / 2.0;
    QPolygonF poly;
    poly << lean(sx,       sy + bev)
         << lean(cx,       sy)
         << lean(sx + sw,  sy + bev)
         << lean(sx + sw,  sy + sh - bev)
         << lean(cx,       sy + sh)
         << lean(sx,       sy + sh - bev);
    return poly;
  };

  p.setPen(Qt::NoPen);

  auto draw = [&](bool isOn, QPolygonF poly) {
    p.setBrush(isOn ? on : off);
    p.drawPolygon(poly);
  };

  draw(seg & 0x01, hPoly(x0, yA, hw, T)); // A – top
  draw(seg & 0x02, vPoly(rx, yF, T, vH)); // B – upper-right
  draw(seg & 0x04, vPoly(rx, yE, T, vH)); // C – lower-right
  draw(seg & 0x08, hPoly(x0, yD, hw, T)); // D – bottom
  draw(seg & 0x10, vPoly(lx, yE, T, vH)); // E – lower-left
  draw(seg & 0x20, vPoly(lx, yF, T, vH)); // F – upper-left
  draw(seg & 0x40, hPoly(x0, yG, hw, T)); // G – middle

  const qreal dotR = T * 0.38;
  const QPointF dotC = lean(rx + T - dotR * 1.2, yD + T - dotR * 1.2);
  p.setBrush((seg & 0x80) ? on : off);
  p.drawEllipse(dotC, dotR, dotR);
}

void IO7Indicator::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(rect(), Qt::black);

  const int n = static_cast<int>(m_digitValues.size());
  if (n == 0)
    return;

  const int margin = qMax(6, qMin(width(), height()) / 18);
  const QRectF area = rect().adjusted(margin, margin, -margin, -margin);
  if (area.width() <= 0 || area.height() <= 0)
    return;

  const qreal digitHFromWidth =
      area.width() / (n * s_digitAspect + (n - 1) * s_gapRatio);
  const qreal digitH = qMax<qreal>(1.0, qMin(area.height(), digitHFromWidth));
  const qreal digitW = digitH * s_digitAspect;
  const qreal gap = qMax<qreal>(2.0, digitH * s_gapRatio);
  const qreal totalW = n * digitW + (n - 1) * gap;
  const qreal startX = area.left() + (area.width() - totalW) / 2.0;
  const qreal startY = area.top() + (area.height() - digitH) / 2.0;

  for (int i = 0; i < n; ++i) {
    const qreal x = startX + i * (digitW + gap);
    drawDigit(painter, qRound(x), qRound(startY), qRound(digitW),
              qRound(digitH), m_digitValues[i]);
  }
}

}
