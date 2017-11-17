#ifndef SHAPE_H
#define SHAPE_H

#include <QBrush>
#include <QGraphicsItem>
#include <QPen>

namespace Graphics {

enum class ShapeType { Block, ALU, MUX };

class Shape : public QGraphicsItem {
   public:
    Shape();

    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget);
    void addInput(QString input) { m_inputs.append(input); }
    void addInput(QStringList input) { m_inputs.append(input); }
    void addOutput(QString output) { m_inputs.append(output); }
    void addOutput(QStringList output) { m_inputs.append(output); }
    void setName(QString name) { m_name = name; }

   private:
    QRectF calculateRect() const;

    ShapeType m_type = ShapeType::Block;
    QString m_name;

    QList<QString> m_inputs;
    QList<QString> m_outputs;

    // drawing constants
    qreal nodeHeight = 10;
    qreal nameFontSize = 14;
    qreal ioFontSize = 10;
};

}  // namespace Graphics

#endif  // SHAPE_H
