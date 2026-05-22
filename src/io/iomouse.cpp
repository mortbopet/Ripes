#include "iomouse.h"
#include "ioregistry.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QWheelEvent>

namespace Ripes {

IOMouse::IOMouse(QWidget *parent) : IOBase(IOType::MOUSE, parent) {
  m_parameters[WIDTH] = IOParam(WIDTH, "Width", 200, true, 100, 800);
  m_parameters[HEIGHT] = IOParam(HEIGHT, "Height", 150, true, 100, 600);

  m_regDescs.push_back(RegDesc{"X", RegDesc::RW::R, 32, X, true});
  m_regDescs.push_back(RegDesc{"Y", RegDesc::RW::R, 32, Y, true});
  m_regDescs.push_back(RegDesc{"LBUTTON", RegDesc::RW::R, 1, LBUTTON, true});
  m_regDescs.push_back(RegDesc{"RBUTTON", RegDesc::RW::R, 1, RBUTTON, true});
  m_regDescs.push_back(RegDesc{"SCROLL", RegDesc::RW::RW, 32, SCROLL, true});

  updateExtraSymbols();
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
}

void IOMouse::updateExtraSymbols() {
  const unsigned w = m_parameters.at(WIDTH).value.toUInt();
  const unsigned h = m_parameters.at(HEIGHT).value.toUInt();
  m_extraSymbols.clear();
  m_extraSymbols.push_back(IOSymbol{"WIDTH", w});
  m_extraSymbols.push_back(IOSymbol{"HEIGHT", h});
}

void IOMouse::parameterChanged(unsigned) {
  updateExtraSymbols();
  updateGeometry();
  update();
}

QString IOMouse::description() const {
  QStringList desc;
  desc << "Tracks mouse position, button state, and scroll within the "
          "widget area.";
  desc << "X, Y = coordinates; LBUTTON, RBUTTON = press state (0/1); "
          "SCROLL = cumulative scroll delta.";
  return desc.join('\n');
}

VInt IOMouse::ioRead(AInt offset, unsigned) {
  switch (offset) {
  case X:
    return m_mouseX;
  case Y:
    return m_mouseY;
  case LBUTTON:
    return m_lButton;
  case RBUTTON:
    return m_rButton;
  case SCROLL:
    return m_scroll;
  }
  return 0;
}

void IOMouse::ioWrite(AInt offset, VInt value, unsigned) {
  switch (offset) {
  case SCROLL:
    m_scroll = value;
    break;
  }
}

void IOMouse::mouseMoveEvent(QMouseEvent *event) {
  const int w = m_parameters.at(WIDTH).value.toInt();
  const int h = m_parameters.at(HEIGHT).value.toInt();
  m_mouseX = qBound(0, event->pos().x(), w - 1);
  m_mouseY = qBound(0, event->pos().y(), h - 1);
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
  const int w = m_parameters.at(WIDTH).value.toInt();
  const int h = m_parameters.at(HEIGHT).value.toInt();
  return QSize(w, h);
}

QSize IOMouse::sizeHint() const { return minimumSizeHint(); }

void IOMouse::reset() {
  m_mouseX = 0;
  m_mouseY = 0;
  m_lButton = 0;
  m_rButton = 0;
  m_scroll = 0;
  update();
}

void IOMouse::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  const int w = m_parameters.at(WIDTH).value.toInt();
  const int h = m_parameters.at(HEIGHT).value.toInt();

  painter.fillRect(rect(), QColor(240, 240, 240));
  painter.fillRect(0, 0, w, h, Qt::white);

  painter.setPen(QPen(Qt::black, 1));
  painter.drawRect(0, 0, w - 1, h - 1);

  painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
  painter.drawLine(m_mouseX, 0, m_mouseX, h);
  painter.drawLine(0, m_mouseY, w, m_mouseY);

  painter.setPen(Qt::NoPen);
  QColor dotColor = Qt::black;
  if (m_lButton)
    dotColor = Qt::red;
  else if (m_rButton)
    dotColor = Qt::blue;
  painter.setBrush(dotColor);
  painter.drawEllipse(QPoint(m_mouseX, m_mouseY), 4, 4);

  painter.setPen(Qt::black);
  const QString info =
      QString("(%1, %2) SCR:%3").arg(m_mouseX).arg(m_mouseY).arg(m_scroll);
  painter.drawText(4, h - 4, info);
}

} // namespace Ripes