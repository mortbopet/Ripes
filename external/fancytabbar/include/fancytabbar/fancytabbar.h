#ifndef FANCYTABBAR_H
#define FANCYTABBAR_H

#include <QFont>
#include <QRect>
#include <QVector>
#include <QWidget>

#include "fancytabbar/fancytab.h"

QT_FORWARD_DECLARE_CLASS(QVariantAnimation)

/*!
 * \brief A vertical icon+label navigation rail, in the spirit of the sidebars
 *   found in modern IDEs and Fluent/macOS navigation views.
 *
 *   The selected item is indicated by a soft, rounded accent "pill" together
 *   with a crisp accent rail hugging the leading edge; the selection slides
 *   smoothly when the active tab changes. Hovered items get a subtle rounded
 *   wash. Colors are derived entirely from the active QPalette, so the rail
 *   follows the application's light/dark theme.
 */
class FancyTabBar : public QWidget {
  Q_OBJECT
  /// Vertical center (in widget coordinates) of the selection highlight.
  /// Driving this via a property lets the selection animate smoothly between
  /// tabs.
  Q_PROPERTY(qreal selectionPos READ selectionPos WRITE setSelectionPos)
public:
  enum Error { SUCESS = 0, INDEX_OUT_OF_RANGE };

  FancyTabBar();
  explicit FancyTabBar(QWidget *widget);
  ~FancyTabBar() override;

  qint32 addFancyTab(QIcon icon, QString text);

  qint32 getActiveIndex() const;
  Error setActiveIndex(qint32 index);

  qreal selectionPos() const { return m_selectionPos; }
  void setSelectionPos(qreal pos);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;

  QRect getTabRect(qint32 index) const;
  QRect getIconRect(qint32 index) const;
  QRect getTextRect(qint32 index) const;
  qint32 getTabYPos(qint32 index) const;
  qreal tabCenterY(qint32 index) const;

  qint32 getTabIndexByPoint(qint32 x, qint32 y) const;

  void drawTabContent(QPainter *painter, qint32 index, bool active);

signals:
  void activeIndexChanged(qint32 index);

private:
  void init();
  /// Animate (or, on first assignment, snap) the selection highlight to the
  /// vertical center of the given tab.
  void moveSelectionTo(qint32 index, bool animate);

  QVector<FancyTab *> tabVector;
  qint32 activeIndex;
  qint32 m_hoverIndex;

  qint32 barWidth;
  qint32 iconSize;
  qint32 textHeight;
  qint32 tabHeight;
  qint32 tabWidth;
  qint32 tabTopSpacing;
  qint32 iconTextGap;

  QFont m_labelFont;

  qreal m_selectionPos;
  bool m_selectionInitialized;
  QVariantAnimation *m_selectionAnim;
};

#endif // FANCYTABBAR_H
