#include "cacheview.h"

#include <QGraphicsSimpleTextItem>
#include <QScrollBar>
#include <QWheelEvent>
#include <algorithm>
#include <qmath.h>

#include "ripessettings.h"

namespace Ripes {

CacheView::CacheView(QWidget *parent) : QGraphicsView(parent) {
  m_zoom = 250;

  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  // Follow the application theme rather than a hardcoded white background.
  setBackgroundBrush(palette().base());
  setInteractive(true);
  setupMatrix();

  // Keep the background in sync with the active light/dark palette.
  connect(ThemeManager::get(), &ThemeManager::themeChanged, this,
          [this] { setBackgroundBrush(palette().base()); });
}

void CacheView::mousePressEvent(QMouseEvent *event) {
  // Panning: middle-drag, or Ctrl+left-drag, moves the scene viewport without
  // triggering block selection.
  const bool startPan = event->button() == Qt::MiddleButton ||
                        (event->button() == Qt::LeftButton &&
                         (event->modifiers() & Qt::ControlModifier));
  if (startPan) {
    m_panning = true;
    m_lastPanPos = event->pos();
    viewport()->setCursor(Qt::ClosedHandCursor);
    event->accept();
    return;
  }

  // If we press on a cache data block, get the address stored for that block
  // and emit a signal indicating that the address was selected through the
  // cache
  const auto viewItems = items(event->pos());
  for (const auto &item : std::as_const(viewItems)) {
    if (auto *textItem = dynamic_cast<QGraphicsSimpleTextItem *>(item)) {
      const QVariant userData = textItem->data(Qt::UserRole);
      if (userData.isValid()) {
        emit cacheAddressSelected(userData.toULongLong());
        break;
      }
    }
  }
  QGraphicsView::mousePressEvent(event);
}

void CacheView::mouseMoveEvent(QMouseEvent *event) {
  if (m_panning) {
    const QPoint delta = event->pos() - m_lastPanPos;
    m_lastPanPos = event->pos();
    ensureNavigableSceneRect();
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    event->accept();
    return;
  }
  QGraphicsView::mouseMoveEvent(event);
}

void CacheView::mouseReleaseEvent(QMouseEvent *event) {
  if (m_panning && (event->button() == Qt::MiddleButton ||
                    event->button() == Qt::LeftButton)) {
    m_panning = false;
    viewport()->setCursor(Qt::ArrowCursor);
    event->accept();
    return;
  }
  QGraphicsView::mouseReleaseEvent(event);
}

void CacheView::ensureNavigableSceneRect() {
  if (scene() == nullptr)
    return;

  const QRectF content = scene()->itemsBoundingRect();
  const QRectF viewportScene = mapToScene(viewport()->rect()).boundingRect();
  QRectF wanted = content.united(viewportScene);

  // Extend by (at least) one content/viewport span in each direction so the
  // view can always be panned past its content and the content can be centered.
  const qreal marginX = std::max(content.width(), viewportScene.width());
  const qreal marginY = std::max(content.height(), viewportScene.height());
  wanted.adjust(-marginX, -marginY, marginX, marginY);

  const QRectF current = sceneRect();
  if (!current.contains(wanted)) {
    // Preserve the current view center across the scene rect change to avoid
    // any visual jump while panning.
    const QPointF center = mapToScene(viewport()->rect().center());
    setSceneRect(current.isEmpty() ? wanted : current.united(wanted));
    centerOn(center);
  }
}

void CacheView::fitScene() {
  scene()->setSceneRect(scene()->itemsBoundingRect());
  fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}

void CacheView::wheelEvent(QWheelEvent *e) {
  if (e->modifiers() & Qt::ControlModifier) {
    if (e->angleDelta().y() > 0)
      zoomIn(6);
    else
      zoomOut(6);
    e->accept();
  } else {
    QGraphicsView::wheelEvent(e);
  }
}

void CacheView::zoomIn(int level) {
  m_zoom += level;
  setupMatrix();
}

void CacheView::zoomOut(int level) {
  m_zoom -= level;
  setupMatrix();
}

void CacheView::setupMatrix() {
  qreal scale = qPow(qreal(2), (m_zoom - 250) / qreal(50));

  QTransform matrix;
  matrix.scale(scale, scale);

  setTransform(matrix);
}

} // namespace Ripes
