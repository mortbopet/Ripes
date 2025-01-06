#ifndef FANCYTABBAR_H
#define FANCYTABBAR_H

#include <QRect>
#include <QVector>
#include <QWidget>

#include "fancytab.h"

/*!
 * \brief Goal of this class is TabBar similar to QtCreator left TabBarr.
 *   TabBar usage is described in example folder.
 */

class FancyTabBar : public QWidget {
  Q_OBJECT
public:
  enum Error { SUCESS = 0, INDEX_OUT_OF_RANGE };

  FancyTabBar();
  explicit FancyTabBar(QWidget *widget);
  virtual ~FancyTabBar();

  qint32 addFancyTab(QIcon icon, QString text);

  qint32 getActiveIndex() const;
  Error setActiveIndex(qint32 index);

protected:
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void enterEvent(QEnterEvent *event);
  void leaveEvent(QEvent *event);

  QRect getTabRect(qint32 index);
  QRect getIconRect(qint32 index);
  QRect getTextRect(qint32 index);
  qint32 getTabYPos(qint32 index);

  qint32 getTabIndexByPoint(qint32 x, qint32 y);

  void drawTabContent(QPainter *painter, qint32 index,
                      bool invertTextColor = false);

signals:
  void activeIndexChanged(qint32 index);

private:
  void init();

  QVector<FancyTab *> tabVector;
  qint32 activeIndex;

  qint32 barWidth;
  qint32 iconSize;
  qint32 textHeight;
  qint32 textWidth;
  qint32 tabHeight;
  qint32 tabWidth;
  qint32 tabTopSpaceing;
  qint32 hower;
};

#endif // FANCYTABBAR_H
