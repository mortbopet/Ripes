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

   private:
    void scaleView(qreal scaleFactor);
    void adjustPositioning();

    void createConnection(Graphics::Shape* source, int index1,
                          Graphics::Shape* dest, int index2);

    QList<QGraphicsItem*> filterAllowedItems(Graphics::Shape* shape,
                                             QList<QGraphicsItem*> items);

    qreal shapeMargin = 10;  // Minimum distance between two shapes
    qreal minConnectionLen = 50;

    QList<Graphics::Connection*> m_connections;
};

#endif  // PIPELINEWIDGET_H
