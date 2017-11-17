#ifndef SHAPE_H
#define SHAPE_H

#include <QBrush>
#include <QGraphicsItem>
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
    void addInput(QStringList input) { m_inputs.append(input); }
    void addOutput(QString output) { m_outputs.append(output); }
    void addOutput(QStringList output) { m_outputs.append(output); }
    void setName(QString name) { m_name = name; }

   private:
    QRectF calculateRect() const;

    ShapeType m_type;
    QString m_name;

    // Extra size that is added onto each dimension of the shape.
    // Used to fine-tune the shape of the widget
    int m_verticalPad;
    int m_horizontalPad;

    QList<QString> m_inputs;
    QList<QString> m_outputs;

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
