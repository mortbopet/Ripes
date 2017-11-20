#ifndef PIPELINEWIDGET_H
#define PIPELINEWIDGET_H

#include <QGraphicsItem>
#include <QGraphicsView>

namespace Graphics {
class Connection;
class Shape;
}

class PipelineWidget : public QGraphicsView {
  Q_OBJECT
 public:
  PipelineWidget(QWidget* parent = nullptr);

  void wheelEvent(QWheelEvent* event);
  void expandToView() {
    fitInView(scene()->sceneRect().adjusted(-10, 0, 10, 0),
              Qt::KeepAspectRatio);
  }

 private:
  void scaleView(qreal scaleFactor);
  void adjustPositioning();

  void createConnection(Graphics::Shape* source, int index1,
                        Graphics::Shape* dest, int index2);
  void createConnection(Graphics::Shape* source, Graphics::Shape* dest,
                        QPointF* sourcePoint, QPointF* destPoint);

  void moveToIO(Graphics::Shape* source, Graphics::Shape* dest,
                QPointF* sourcePoint, QPointF* destPointm,
                int connectionLength = minConnectionLen);

  QList<QGraphicsItem*> filterAllowedItems(Graphics::Shape* shape,
                                           QList<QGraphicsItem*> items);

  qreal shapeMargin = 15;  // Minimum distance between two shapes
  constexpr static qreal minConnectionLen = 50;

  QList<Graphics::Connection*> m_connections;
};

#endif  // PIPELINEWIDGET_H
