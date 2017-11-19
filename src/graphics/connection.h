#ifndef CONNECTION_H
#define CONNECTION_H

#include <QGraphicsItem>

namespace Graphics {
class Shape;

enum class ValueDrawPos { Source, Middle, Destination };

/*
    Class for drawing connections and signal values between shapes in the
   pipeline view.
*/
class Connection : public QGraphicsItem {
   public:
    Connection(Shape *source, QPointF *sourcePoint, Shape *dest,
               QPointF *destPoint);
    void setValueDrawPos(ValueDrawPos pos) { m_valuePos = pos, update(); }

    void setValue(int value) { m_value = value; }
    void showValue(bool state) {
        m_showValue = state;
        update();
    }
    QPair<QPointF, QPointF> getPoints() const;
    QPair<Shape *, Shape *> getShapes() const {
        return QPair<Shape *, Shape *>(m_source, m_dest);
    }

   protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

   private:
    // A list of IO points, which all connect to this widget -
    // used to form attachment points to this connection
    QList<QPointF *> m_connectsToThis;

    // Variables related to the current value of the connection (signal value)
    bool m_showValue = false;
    int m_value;
    ValueDrawPos m_valuePos = ValueDrawPos::Source;

    int m_arrowSize = 10;

    Shape *m_source;
    Shape *m_dest;
    QPointF *m_sourcePointPtr;
    QPointF *m_destPointPtr;
};

}  // namespace Graphics

#endif  // CONNECTION_H
