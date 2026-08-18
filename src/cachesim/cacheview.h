#pragma once

#include <QGraphicsView>
#include <QPoint>

#include "isa/isa_types.h"

namespace Ripes {

class CacheView : public QGraphicsView {
  Q_OBJECT
public:
  CacheView(QWidget *parent);
  void fitScene();

protected:
  void wheelEvent(QWheelEvent *) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

signals:
  void cacheAddressSelected(Ripes::AInt);

private slots:
  void setupMatrix();
  void zoomIn(int level = 1);
  void zoomOut(int level = 1);

private:
  /// Grows the scene rect so it always extends beyond both the content and the
  /// current viewport, keeping the view freely pannable in every direction.
  /// The current view center is preserved across the change.
  void ensureNavigableSceneRect();

  qreal m_zoom;
  bool m_panning = false;
  QPoint m_lastPanPos;
};

} // namespace Ripes
