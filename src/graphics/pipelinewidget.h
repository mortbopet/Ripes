#ifndef PIPELINEWIDGET_H
#define PIPELINEWIDGET_H

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPair>

#include "defines.h"
#include "pipeline.h"

namespace Graphics {

class Connection;
class Text;
class Shape;
}  // namespace Graphics

#define CONNECTION_Z 1

typedef QPair<Graphics::Shape*, int> ShapePair;

class PipelineWidget : public QGraphicsView {
    Q_OBJECT
public:
    PipelineWidget(QWidget* parent = nullptr);

    void wheelEvent(QWheelEvent* event) override;
    void expandToView() { fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio); }
    void displayAllValues(bool state);
    void zoomIn();
    void zoomOut();

public slots:
    void update();
    void stageTextChanged(Stage stage, const QString& text, QColor col);

signals:
    void displayAllValuesSig(bool state);

private:
    void notifyLabelsGeometryChange();
    void scaleView(qreal scaleFactor);
    void adjustPositioning();
    void setSignal(Graphics::Connection* conn, SignalBase* sig);

    Graphics::Connection* createConnection(Graphics::Shape* source, int index1, Graphics::Shape* dest, int index2,
                                           SignalBase* sig = nullptr);
    Graphics::Connection* createConnection(Graphics::Shape* source, Graphics::Shape* dest, QPointF* sourcePoint,
                                           QPointF* destPoint, SignalBase* sig = nullptr);
    Graphics::Connection* createConnection(Graphics::Shape* source, int index1, QList<ShapePair> dests);

    void moveToIO(Graphics::Shape* source, Graphics::Shape* dest, QPointF* sourcePoint, QPointF* destPointm,
                  int connectionLength = minConnectionLen);

    QList<QGraphicsItem*> filterAllowedItems(Graphics::Shape* shape, QList<QGraphicsItem*> items);

    constexpr static qreal shapeMargin = 15;  // Minimum distance between two shapes
    constexpr static qreal stateRegHeight = 500;
    static qreal spaceBetweenStateRegs;
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

    Pipeline* m_pipelinePtr;

    // Vector of Shape* where each item gets its update() function called whenever stepping the simulator.
    std::vector<QGraphicsItem*> m_animatedItems;
};

#endif  // PIPELINEWIDGET_H
