#include "fancytabbar.h"

#include <QDebug>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>

/*!
 * \brief FancyTabBar::FancyTabBar is basic constructor if you want to
 *   create stand alone window with menu.
 */
FancyTabBar::FancyTabBar() : QWidget(NULL) { init(); }

/*!
 * \brief FancyTabBar::FancyTabBar is used for creating embedded fancyTabBar
 *   into some window.
 * \param widget
 * \see QWidget documentation.
 */
FancyTabBar::FancyTabBar(QWidget *widget) : QWidget(widget) { init(); }

/*!
 * \brief FancyTabBar::~FancyTabBar delete allocated memory (free tabVector).
 */
FancyTabBar::~FancyTabBar() {
  for (int i = 0; i < tabVector.size(); i++) {
    delete tabVector.at(i);
  }
  tabVector.clear();
}

/*!
 * \brief FancyTabBar::addFancyTab add fancy tab to tab bar. New tab will
 *    contain passed icon and taxst as capption.
 * \param icon will be showed as tab icon. It is similar to tool bar icon.
 * \param text will be showed as captoin.
 * \return return id (index) of tab. This value is returned by signal. Indexes
 *    are starting at 0 and they are incremented with each new tab.
 * \see FancyTabBar::activeIndexChanged()
 */
qint32 FancyTabBar::addFancyTab(QIcon icon, QString text) {
  FancyTab *fancyTab = new FancyTab(icon, text);
  tabVector.append(fancyTab);
  setMinimumHeight((tabHeight * tabVector.size()) + 2);
  return tabVector.size() - 1;
}

/*!
 * \brief FancyTabBar::getActiveIndex return index of active tab.
 * \return index of active tab.
 */
qint32 FancyTabBar::getActiveIndex() const { return activeIndex; }

/*!
 * \brief FancyTabBar::setActiveIndex set active tab. This function is mostly
 *    used on start up to set which tab is active.
 * \param index tab index
 * \return FancyTabBar::SUCESS if sucesfuly set.
 * \return FancyTabBar::INDEX_OUT_OF_RANGE if index is out of possible range.
 */
FancyTabBar::Error FancyTabBar::setActiveIndex(qint32 index) {
  if (index >= tabVector.size() || index < 0)
    return INDEX_OUT_OF_RANGE;

  activeIndex = index;
  emit activeIndexChanged(activeIndex);

  return SUCESS;
}

/*!
 * \brief FancyTabBar::paintEvent this function is called if the FancyTabBar
 *   should be repainted.
 *   <H1>Drawing process</H1>
 *     -# Firt of all the frame is drawen with gradient as backgroud.
 *     -# Hovered tabs background is drawen.
 *     -# All tabs are drawen except current active tab.
 *     -# If some tab is active then
 *        -# draw the active backgroud.
 *        -# draw active tab
 *        -# draw the borders around the active tab (borders are drowen in to
 *           neighbored tabs!!!).
 * \param event see qt documentation for more details about this parameter.
 */
void FancyTabBar::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);

  // Reduced size is size reduced by one becouse we need to draw from 0
  // so size - 1. That how all arrays works ;)
  QSize reducedSize = this->size() - QSize(1, 1);

  // Draw separation rect
  // Rest of parameters are zeros becouse we want to do only vertical
  // gradient.

  QLinearGradient linearGradient(0, 0, reducedSize.width(), 0);
  linearGradient.setColorAt(0, QColor(64, 64, 64));
  linearGradient.setColorAt(1, QColor(130, 130, 130));
  // painter.setBrush(linearGradient);

  painter.setBrush(QColor(0x656565));
  painter.setPen(QColor(49, 49, 49));
  painter.drawRect(0, 0, reducedSize.width(), reducedSize.height());

  if (hower >= 0 && hower != activeIndex) {
    QLinearGradient lineaGradientHover =
        QLinearGradient(0, 0, reducedSize.width(), 0);
    lineaGradientHover.setColorAt(0, QColor(200, 200, 200, 20));
    lineaGradientHover.setColorAt(0.5, QColor(200, 200, 200, 100));
    lineaGradientHover.setColorAt(1, QColor(200, 200, 200, 20));
    painter.setPen(Qt::NoPen);
    painter.setBrush(lineaGradientHover);
    painter.drawRect(getTabRect(hower));
  }

  for (int i = 0, y = 0; i < tabVector.size(); i++, y += 50) {
    // Active tab must be drawen last because it will draw into neighbours
    // tabs... This is most easy way how to draw specific frames
    if (activeIndex != i) {
      drawTabContent(&painter, i);
    }
  }

  // Draw activ tab if any
  // Will draw into neighbors tab!!!
  if (activeIndex >= 0) {
    // Make gradient more lighter. This indicate active tab
    QColor color1(170, 170, 170);
    QColor color2(233, 233, 233);
    linearGradient.setColorAt(0, color1);
    linearGradient.setColorAt(1, color2);

    // painter.setBrush(linearGradient);
    painter.setBrush(palette().color(QWidget::backgroundRole()));
    painter.setPen(Qt::NoPen);
    painter.drawRect(getTabRect(activeIndex));

    drawTabContent(&painter, activeIndex, true);

    painter.setPen(QColor(49, 49, 49));

    QRect r = getTabRect(activeIndex).adjusted(-1, -1, 1, 1);
    // Draw top line
    painter.drawLine(r.topLeft(), r.topRight());
    // Draw bottom line
    painter.drawLine(r.bottomLeft(), r.bottomRight());

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush());
    color1.setRgb(100, 100, 100);
    color2.setRgb(150, 150, 150);
    linearGradient.setColorAt(0, color1);
    linearGradient.setColorAt(1, color2);
    painter.setBrush(linearGradient);
    r.adjust(1, -1, 0, 1);

    // Draw top line
    painter.drawRect(r.left(), r.top(), r.right() - 1, 1);
    // Draw bottom line
    painter.drawRect(r.left(), r.bottom(), r.right() - 1, 1);

    // remove side line
    painter.setBrush(palette().color(QWidget::backgroundRole()));
    painter.setPen(palette().color(QWidget::backgroundRole()));
    painter.drawLine(r.right(), r.top() + 2, r.right(), r.bottom() - 2);
  }
}

/*!
 * \brief FancyTabBar::mouseReleaseEvent if mouse release event is trigered.
 *   Active tabe must emit signal to let the rest of app know that the index
 *   was changed.
 * \param event see qt documentation for more details about this parameter.
 */
void FancyTabBar::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    qint32 ret =
        getTabIndexByPoint(event->position().x(), event->position().y());

    // If non of the tabs is clicked dont change the curent activeIndex.
    if (ret != -1)
      activeIndex = ret;

    update();

    emit activeIndexChanged(activeIndex);
  }
  QWidget::mouseReleaseEvent(event);
}

/*!
 * \brief FancyTabBar::mouseMoveEvent whan the cursor is inside FancyTabBar area
 *   we need to update howered tab index if the cursor move.
 * \param event see qt documentation for more details about this parameter.
 */
void FancyTabBar::mouseMoveEvent(QMouseEvent *event) {
  QWidget::mouseMoveEvent(event);
  hower = getTabIndexByPoint(event->position().x(), event->position().y());
  update();
}

/*!
 * \brief FancyTabBar::enterEvent make hower tab which is under cursor when the
 *   cursor enter FancyTabBar area.
 * \param event see qt documentation for more details about this parameter.
 */
void FancyTabBar::enterEvent(QEnterEvent *event) {
  QEnterEvent *enterEvent = static_cast<QEnterEvent *>(event);
  hower = getTabIndexByPoint(enterEvent->position().x(),
                             enterEvent->position().y());
  update();
}

/*!
 * \brief FancyTabBar::leaveEvent make the hover over some tab disapare.
 * \param event see qt documentation for more details about this parameter.
 */
void FancyTabBar::leaveEvent(QEvent *event) {
  Q_UNUSED(event);
  // disable hower
  hower = -1;
  // repaint
  update();
}

/*!
 * \brief FancyTabBar::getTabRect returns position and size of given tab
 *   index.
 * \param index index of the tab.
 */
QRect FancyTabBar::getTabRect(qint32 index) {
  qint32 tabPos = getTabYPos(index);
  return QRect(1, tabPos, tabWidth, tabHeight);
}

/*!
 * \brief FancyTabBar::getIconRect returns position and size of icon of given
 *   tab index.
 * \param index index of the tab
 */
QRect FancyTabBar::getIconRect(qint32 index) {
  qint32 iconPos = getTabYPos(index) + tabTopSpaceing;
  return QRect((tabWidth - iconSize) / 2, iconPos, iconSize, iconSize);
}

/*!
 * \brief FancyTabBar::getTextRect returns position and size of text area of
 *   given tab index.
 * \param index index of the tab
 */
QRect FancyTabBar::getTextRect(qint32 index) {
  qint32 textPos = getTabYPos(index) + tabTopSpaceing + iconSize;
  return QRect(1, textPos, tabWidth, textHeight);
}

/*!
 * \brief FancyTabBar::getTabYPos return Y position of give tab index. This
 *   function is mostly used by all another internal position functions.
 * \param index index of the tab
 */
qint32 FancyTabBar::getTabYPos(qint32 index) { return (tabHeight * index) + 1; }

/*!
 * \brief FancyTabBar::getTabIndexByPoint take poin in the 2D space and tels
 *   if it is inside of some tab. Returned value is tab index if some tab is
 *   found.
 * \param x x position of point.
 * \param y y position of point.
 * \return tab index if success. -1 otherwise.
 */
qint32 FancyTabBar::getTabIndexByPoint(qint32 x, qint32 y) {
  if (x < 1 || x > tabWidth)
    return -1;
  if (y > ((tabVector.size() * tabHeight) - 2))
    return -1;

  return (y - 1) / tabHeight;
}

/*!
 * \brief FancyTabBar::drawTabContent function which draw tab context. That
 *   mean this function draw tab icon and tab text on position returned by
 *   getTextRect() and getIconRect()
 * \param painter is painter passed from paintEvent
 * \param index is index of tab inside FancyTabBar
 * \param invertTextColor tels to function is text color should by inverted.
 *   This feature is usefull whan some tab is selected.
 */
void FancyTabBar::drawTabContent(QPainter *painter, qint32 index,
                                 bool invertTextColor) {
  if (invertTextColor)
    painter->setPen(QColor(0x333333));
  else
    painter->setPen(QColor(0xd6d6d6));

  QFont font = painter->font();
  font.setBold(true);
  font.setPixelSize(10);
  painter->setFont(font);

  QPixmap pixmap = tabVector[index]->m_icon.pixmap(iconSize, iconSize);

  QRect iconRect = getIconRect(index);
  QRect textRect = getTextRect(index);

  painter->drawPixmap(iconRect.topLeft(), pixmap);
  painter->drawText(textRect, Qt::AlignBottom | Qt::AlignHCenter,
                    tabVector[index]->m_text);
}

/*!
 * \brief FancyTabBar::init init all variable inside FancyTabBar class.
 *   Must be called in all constructors....
 */
void FancyTabBar::init() {
  activeIndex = -1;

  barWidth = 80;

  setMaximumWidth(barWidth);
  setMinimumWidth(barWidth);

  iconSize = 36;
  textHeight = 10;
  tabWidth = barWidth - 2; // Two are substraceted because there are two
                           // pixels of frame.
  textWidth = tabWidth;
  tabHeight = 55;
  // on the left side
  tabTopSpaceing = 5;

  setMouseTracking(true);
  hower = -1;
}
