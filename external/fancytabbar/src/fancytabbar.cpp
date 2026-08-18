#include "fancytabbar/fancytabbar.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

namespace {
// A little breathing room at the top and bottom of the rail.
constexpr int kTopMargin = 6;
// Duration of the selection slide animation.
constexpr int kSelectionAnimMs = 180;
} // namespace

FancyTabBar::FancyTabBar() : QWidget(nullptr) { init(); }

FancyTabBar::FancyTabBar(QWidget *widget) : QWidget(widget) { init(); }

FancyTabBar::~FancyTabBar() {
  for (int i = 0; i < tabVector.size(); i++) {
    delete tabVector.at(i);
  }
  tabVector.clear();
}

qint32 FancyTabBar::addFancyTab(QIcon icon, QString text) {
  FancyTab *fancyTab = new FancyTab(icon, text);
  tabVector.append(fancyTab);
  setMinimumHeight((tabHeight * tabVector.size()) + 2 * kTopMargin);
  return tabVector.size() - 1;
}

qint32 FancyTabBar::getActiveIndex() const { return activeIndex; }

FancyTabBar::Error FancyTabBar::setActiveIndex(qint32 index) {
  if (index >= tabVector.size() || index < 0)
    return INDEX_OUT_OF_RANGE;

  activeIndex = index;
  moveSelectionTo(index, /*animate=*/m_selectionInitialized);
  emit activeIndexChanged(activeIndex);
  update();

  return SUCESS;
}

void FancyTabBar::setSelectionPos(qreal pos) {
  m_selectionPos = pos;
  update();
}

void FancyTabBar::moveSelectionTo(qint32 index, bool animate) {
  if (index < 0 || index >= tabVector.size())
    return;
  const qreal target = tabCenterY(index);
  m_selectionAnim->stop();
  if (!animate || !m_selectionInitialized) {
    m_selectionInitialized = true;
    setSelectionPos(target);
    return;
  }
  m_selectionAnim->setStartValue(m_selectionPos);
  m_selectionAnim->setEndValue(target);
  m_selectionAnim->start();
}

void FancyTabBar::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  const QPalette pal = palette();
  const bool darkTheme = pal.color(QPalette::Window).lightness() < 128;

  // --- Background rail ---------------------------------------------------
  // A flat surface, a hair different from the window so the rail reads as its
  // own region, with a single hairline separator on the trailing edge instead
  // of a hard box border.
  const QColor barBackground = darkTheme
                                   ? pal.color(QPalette::Window).lighter(118)
                                   : pal.color(QPalette::Window).darker(106);
  painter.fillRect(rect(), barBackground);
  QColor separator = pal.color(QPalette::Mid);
  separator.setAlpha(darkTheme ? 120 : 160);
  painter.setPen(separator);
  painter.drawLine(width() - 1, 0, width() - 1, height());

  const qreal pillMargin = 8;
  const qreal pillW = tabWidth - 2 * pillMargin;
  const qreal pillH = tabHeight - 10;
  const qreal pillRadius = 12;

  // --- Hover wash --------------------------------------------------------
  if (m_hoverIndex >= 0 && m_hoverIndex != activeIndex) {
    QColor hover = pal.color(QPalette::WindowText);
    hover.setAlpha(darkTheme ? 28 : 22);
    const qreal cy = tabCenterY(m_hoverIndex);
    const QRectF r(pillMargin, cy - pillH / 2, pillW, pillH);
    painter.setPen(Qt::NoPen);
    painter.setBrush(hover);
    painter.drawRoundedRect(r, pillRadius, pillRadius);
  }

  // --- Selection: soft accent pill + leading accent rail -----------------
  if (activeIndex >= 0) {
    const QColor accent = pal.color(QPalette::Highlight);

    QColor pill = accent;
    pill.setAlpha(darkTheme ? 60 : 42);
    const QRectF pillRect(pillMargin, m_selectionPos - pillH / 2, pillW, pillH);
    painter.setPen(Qt::NoPen);
    painter.setBrush(pill);
    painter.drawRoundedRect(pillRect, pillRadius, pillRadius);

    // Leading indicator rail: a short rounded bar hugging the left edge.
    const qreal railW = 3.5;
    const qreal railH = tabHeight * 0.42;
    const QRectF railRect(2.5, m_selectionPos - railH / 2, railW, railH);
    painter.setBrush(accent);
    painter.drawRoundedRect(railRect, railW / 2, railW / 2);
  }

  // --- Tab contents ------------------------------------------------------
  for (int i = 0; i < tabVector.size(); i++)
    drawTabContent(&painter, i, i == activeIndex);
}

void FancyTabBar::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    qint32 ret =
        getTabIndexByPoint(event->position().x(), event->position().y());

    if (ret != -1 && ret != activeIndex) {
      activeIndex = ret;
      moveSelectionTo(ret, /*animate=*/true);
      emit activeIndexChanged(activeIndex);
    }
    update();
  }
  QWidget::mousePressEvent(event);
}

void FancyTabBar::mouseMoveEvent(QMouseEvent *event) {
  QWidget::mouseMoveEvent(event);
  const qint32 h =
      getTabIndexByPoint(event->position().x(), event->position().y());
  if (h != m_hoverIndex) {
    m_hoverIndex = h;
    update();
  }
}

void FancyTabBar::enterEvent(QEnterEvent *event) {
  m_hoverIndex =
      getTabIndexByPoint(event->position().x(), event->position().y());
  update();
}

void FancyTabBar::leaveEvent(QEvent *event) {
  Q_UNUSED(event);
  m_hoverIndex = -1;
  update();
}

QRect FancyTabBar::getTabRect(qint32 index) const {
  return QRect(0, getTabYPos(index), tabWidth, tabHeight);
}

QRect FancyTabBar::getIconRect(qint32 index) const {
  const qint32 iconY = getTabYPos(index) + tabTopSpacing;
  return QRect((tabWidth - iconSize) / 2, iconY, iconSize, iconSize);
}

QRect FancyTabBar::getTextRect(qint32 index) const {
  const qint32 textY =
      getTabYPos(index) + tabTopSpacing + iconSize + iconTextGap;
  return QRect(0, textY, tabWidth, textHeight);
}

qint32 FancyTabBar::getTabYPos(qint32 index) const {
  return kTopMargin + (tabHeight * index);
}

qreal FancyTabBar::tabCenterY(qint32 index) const {
  return getTabYPos(index) + tabHeight / 2.0;
}

qint32 FancyTabBar::getTabIndexByPoint(qint32 x, qint32 y) const {
  if (x < 0 || x > tabWidth)
    return -1;
  const qint32 rel = y - kTopMargin;
  if (rel < 0)
    return -1;
  const qint32 index = rel / tabHeight;
  if (index >= tabVector.size())
    return -1;
  return index;
}

void FancyTabBar::drawTabContent(QPainter *painter, qint32 index, bool active) {
  const QPalette pal = palette();

  // Icons are left in their natural colors (several are meaningful pictograms);
  // inactive tabs are dimmed slightly to establish hierarchy.
  const QPixmap pixmap = tabVector[index]->m_icon.pixmap(iconSize, iconSize);
  painter->save();
  painter->setOpacity(active ? 1.0 : 0.82);
  painter->drawPixmap(getIconRect(index).topLeft(), pixmap);
  painter->restore();

  QFont font = m_labelFont;
  font.setWeight(active ? QFont::DemiBold : QFont::Normal);
  painter->setFont(font);

  QColor fg = pal.color(QPalette::WindowText);
  if (!active)
    fg.setAlpha(160);
  painter->setPen(fg);
  painter->drawText(getTextRect(index), Qt::AlignHCenter | Qt::AlignVCenter,
                    tabVector[index]->m_text);
}

void FancyTabBar::init() {
  activeIndex = -1;
  m_hoverIndex = -1;

  barWidth = 76;
  iconSize = 30;
  tabTopSpacing = 12;
  iconTextGap = 5;

  m_labelFont = font();
  if (m_labelFont.pointSizeF() > 0)
    m_labelFont.setPointSizeF(m_labelFont.pointSizeF() * 0.9);
  else
    m_labelFont.setPixelSize(11);
  textHeight = QFontMetrics(m_labelFont).height();

  tabWidth = barWidth;
  tabHeight = tabTopSpacing + iconSize + iconTextGap + textHeight + 12;

  setMaximumWidth(barWidth);
  setMinimumWidth(barWidth);

  m_selectionPos = 0;
  m_selectionInitialized = false;
  m_selectionAnim = new QVariantAnimation(this);
  m_selectionAnim->setDuration(kSelectionAnimMs);
  m_selectionAnim->setEasingCurve(QEasingCurve::InOutCubic);
  connect(m_selectionAnim, &QVariantAnimation::valueChanged, this,
          [this](const QVariant &v) { setSelectionPos(v.toReal()); });

  setMouseTracking(true);
}
