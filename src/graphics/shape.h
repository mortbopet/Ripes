#ifndef SHAPE_H
#define SHAPE_H

#include <QBrush>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>

namespace Graphics {
enum class ShapeType { Block, ALU, MUX };

class Shape : public QGraphicsItem {
   public:
    Shape(ShapeType type = ShapeType::Block, int verticalPad = 0,
          int horizontalPad = 0);

    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget);
    void addInput(QString input);
    void addInput(QStringList input);
    void addOutput(QString output);
    void addOutput(QStringList output);
    void setName(QString name);

   private:
    void calculateRect();
    QRectF m_rect;
    bool m_hasChanged = true;  // Flag is set whenever the item has changed (ie.
                               // when new descriptors have been added)

    QPainterPath drawALUPath(QRectF boundingRect) const;

    ShapeType m_type;
    QString m_name;

    // Extra size that is added onto each dimension of the shape.
    // Used to fine-tune the shape of the widget
    int m_verticalPad;
    int m_horizontalPad;

    QList<QString> m_inputs;
    QList<QString> m_outputs;

    QList<QPointF> m_inputPoints;
    QList<QPointF> m_outputPoints;

    // Top- and bottom center-points of the shape
    // Can be used to affix labels to shapes in the graphics scene
    QPointF m_topPoint;
    QPointF m_bottomPoint;

    // drawing constants
    qreal nodeHeight = 20;
    qreal nameFontSize = 10;
    qreal ioFontSize = 8;
    qreal nodePadding = 5;  // padding between each text descriptor for a node
    qreal sidePadding =
      3;  // padding between an IO description and the side of the shape
};

}  // namespace Graphics

#endif  // SHAPE_H
