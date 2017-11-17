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
  Connection(Shape *source, Shape *dest);
  void setValueDrawPos(ValueDrawPos pos) { m_valuePos = pos, update(); }

  void setValue(int value) { m_value = value; }
  void showValue(bool state) {
    m_showValue = state;
    update();
  }
  Shape *getSource() const { return m_source; }
  Shape *getDest() const { return m_dest; }

protected:
  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

private:
  Shape *m_source;
  Shape *m_dest;

  // Variables related to the current value of the connection (signal value)
  bool m_showValue = false;
  int m_value;
  ValueDrawPos m_valuePos = ValueDrawPos::Source;

  QPointF m_sourcePoint;
  QPointF m_destPoint;
};

} // namespace Graphics

#endif // CONNECTION_H
