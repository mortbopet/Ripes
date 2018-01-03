#ifndef PIPELINEWIDGET_H
#define PIPELINEWIDGET_H

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPair>

#include "defines.h"

namespace Graphics {
class Connection;
class Text;
class Shape;
}  // namespace Graphics

typedef QPair<Graphics::Shape*, int> ShapePair;

class PipelineWidget : public QGraphicsView {
    Q_OBJECT
public:
    PipelineWidget(QWidget* parent = nullptr);

    void wheelEvent(QWheelEvent* event);
    void expandToView() { fitInView(scene()->sceneRect().adjusted(-30, 0, 10, 0), Qt::KeepAspectRatio); }
    void displayAllValues(bool state);

public slots:
    void update();
    void stageTextChanged(Stage stage, const QString& text);

signals:
    void displayAllValuesSig(bool state);

private:
    void scaleView(qreal scaleFactor);
    void adjustPositioning();

    Graphics::Connection* createConnection(Graphics::Shape* source, int index1, Graphics::Shape* dest, int index2);
    Graphics::Connection* createConnection(Graphics::Shape* source, Graphics::Shape* dest, QPointF* sourcePoint,
                                           QPointF* destPoint);
    Graphics::Connection* createConnection(Graphics::Shape* source, int index1, QList<ShapePair> dests);

    void moveToIO(Graphics::Shape* source, Graphics::Shape* dest, QPointF* sourcePoint, QPointF* destPointm,
                  int connectionLength = minConnectionLen);

    QList<QGraphicsItem*> filterAllowedItems(Graphics::Shape* shape, QList<QGraphicsItem*> items);

    constexpr static qreal shapeMargin = 15;  // Minimum distance between two shapes
    constexpr static qreal stateRegHeight = 500;
    constexpr static qreal spaceBetweenStateRegs = 350;
    constexpr static qreal minConnectionLen = 30;

    QList<Graphics::Connection*> m_connections;

    // Shape pointers
    Graphics::Shape* ifid;
    Graphics::Shape* idex;
    Graphics::Shape* exmem;
    Graphics::Shape* memwb;

    // Text pointers
    Graphics::Text* if_instr;
    Graphics::Text* id_instr;
    Graphics::Text* ex_instr;
    Graphics::Text* mem_instr;
    Graphics::Text* wb_instr;
};

#endif  // PIPELINEWIDGET_H
