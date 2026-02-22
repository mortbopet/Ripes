#include "iomouse.h"
#include "ioregistry.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QWheelEvent>

namespace Ripes {

IOMouse::IOMouse(QWidget *parent) : IOBase(IOType::MOUSE, parent) {
  m_regDescs.push_back(RegDesc{"X", RegDesc::RW::R, 32, X * 4, true});
  m_regDescs.push_back(RegDesc{"Y", RegDesc::RW::R, 32, Y * 4, true});
  m_regDescs.push_back(
      RegDesc{"LBUTTON", RegDesc::RW::R, 1, LBUTTON * 4, true});
  m_regDescs.push_back(
      RegDesc{"RBUTTON", RegDesc::RW::R, 1, RBUTTON * 4, true});
  m_regDescs.push_back(RegDesc{"SCROLL", RegDesc::RW::RW, 32, SCROLL * 4, true});

  m_extraSymbols.push_back(IOSymbol{"WIDTH", m_width});
  m_extraSymbols.push_back(IOSymbol{"HEIGHT", m_height});

  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
}


void IOMouse::reset() {
  m_mouseX = 0;
  m_mouseY = 0;
  m_lButton = 0;
  m_rButton = 0;
  m_scroll = 0;
}


QString IOMouse::description() const {
  return "Mouse: X, Y, LBUTTON, RBUTTON, SCROLL";
}

VInt IOMouse::ioRead(AInt offset, unsigned) {
  switch (offset) {
  case X * 4:
    return m_mouseX;
  case Y * 4:
    return m_mouseY;
  case LBUTTON * 4:
    return m_lButton;
  case RBUTTON * 4:
    return m_rButton;
  case SCROLL * 4:
    return m_scroll;
  }
  return 0;
}

void IOMouse::ioWrite(AInt offset, VInt value, unsigned) {
  if (offset == SCROLL * 4) {
    m_scroll = value;
  }
}

void IOMouse::mouseMoveEvent(QMouseEvent *event) {
  m_mouseX = event->pos().x();
  m_mouseY = event->pos().y();
  if (m_mouseX < 0)
    m_mouseX = 0;
  if (m_mouseY < 0)
    m_mouseY = 0;
  if (m_mouseX >= (int)m_width)
    m_mouseX = m_width - 1;
  if (m_mouseY >= (int)m_height)
    m_mouseY = m_height - 1;
  emit scheduleUpdate();
}

void IOMouse::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton)
    m_lButton = 1;
  if (event->button() == Qt::RightButton)
    m_rButton = 1;
  emit scheduleUpdate();
}

void IOMouse::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton)
    m_lButton = 0;
  if (event->button() == Qt::RightButton)
    m_rButton = 0;
  emit scheduleUpdate();
}

void IOMouse::wheelEvent(QWheelEvent *event) {
  m_scroll += event->angleDelta().y() / 120;
  emit scheduleUpdate();
}

QSize IOMouse::minimumSizeHint() const {
  return QSize(m_width, m_height);
}

void IOMouse::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  p.fillRect(rect(), QColor(245, 245, 245));

  QPen gridPen(QColor(200, 200, 200));
  gridPen.setWidth(1);
  p.setPen(gridPen);
  for (int x = 0; x < (int)m_width; x += m_width / 4)
    p.drawLine(x, 0, x, m_height);
  for (int y = 0; y < (int)m_height; y += m_height / 4)
    p.drawLine(0, y, m_width, y);

  QPen crossPen(QColor(0, 180, 180));
  crossPen.setWidth(1);
  p.setPen(crossPen);
  p.drawLine(0, m_mouseY, m_width, m_mouseY);
  p.drawLine(m_mouseX, 0, m_mouseX, m_height);

  p.setBrush(QColor(0, 180, 180));
  p.setPen(QPen(QColor(255, 255, 255), 2));
  p.drawEllipse(QPoint(m_mouseX, m_mouseY), 5, 5);

  QString label = "(" + QString::number(m_mouseX) + ", " + QString::number(m_mouseY) + ")";
  QFont font = p.font();
  font.setFamily("Consolas");
  font.setPointSize(10);
  p.setFont(font);
  p.setPen(QColor(0, 180, 180));

  int textX = m_mouseX + 10;
  int textY = m_mouseY - 8;
  QFontMetrics fm(font);
  int textW = fm.horizontalAdvance(label);
  if (textX + textW > (int)m_width)
    textX = m_mouseX - textW - 10;
  if (textY - fm.height() < 0)
    textY = m_mouseY + fm.height() + 4;

  p.drawText(textX, textY, label);

  p.setPen(QPen(QColor(180, 180, 180), 1));
  p.setBrush(Qt::NoBrush);
  p.drawRect(0, 0, m_width - 1, m_height - 1);

  p.end();
}

} // namespace Ripes
