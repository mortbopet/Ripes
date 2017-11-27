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
    void expandToView() { fitInView(scene()->sceneRect().adjusted(-10, 0, 10, 0), Qt::KeepAspectRatio); }
    void displayAllValues(bool state);

signals:
    void displayAllValuesSig(bool state);

private:
    void scaleView(qreal scaleFactor);
    void adjustPositioning();

    Graphics::Connection* createConnection(Graphics::Shape* source, int index1, Graphics::Shape* dest, int index2);
    Graphics::Connection* createConnection(Graphics::Shape* source, Graphics::Shape* dest, QPointF* sourcePoint,
                                           QPointF* destPoint);

    void moveToIO(Graphics::Shape* source, Graphics::Shape* dest, QPointF* sourcePoint, QPointF* destPointm,
                  int connectionLength = minConnectionLen);

    QList<QGraphicsItem*> filterAllowedItems(Graphics::Shape* shape, QList<QGraphicsItem*> items);

    constexpr static qreal shapeMargin           = 15;  // Minimum distance between two shapes
    constexpr static qreal stateRegHeight        = 500;
    constexpr static qreal spaceBetweenStateRegs = 350;
    constexpr static qreal minConnectionLen      = 30;

    QList<Graphics::Connection*> m_connections;

    // Shape pointers
    Graphics::Shape* ifid;
    Graphics::Shape* idex;
    Graphics::Shape* exmem;
    Graphics::Shape* memwb;
};

#endif  // PIPELINEWIDGET_H
