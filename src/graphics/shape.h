#ifndef SHAPE_H
#define SHAPE_H

#include <QBrush>
#include <QGraphicsItem>
#include <QPen>

namespace Graphics {

class Shape : public QGraphicsItem {
  Q_OBJECT
public:
  Shape();

private:
  QSize m_size; // height, width

  QString m_name;
  QList<QPair<QPoint, QString>> m_inputPoints;
  QList<QPair<QPoint, QString>> m_outputPoints;

  QPen m_pen;
  QBrush m_brush;
};

class Rectangle : public Shape {

protected:
  // void paintEvent(QPaintEvent *event);
};

class ALU : public Shape {};
}

#endif // SHAPE_H
