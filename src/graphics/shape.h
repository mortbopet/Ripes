#ifndef SHAPE_H
#define SHAPE_H

#include <QBrush>
#include <QFont>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>

#include "pipelinewidget.h"

namespace Graphics {
class Connection;
enum class ShapeType { Block, ALU, MUX, Static };
enum class Stage { IF = 1, ID = 2, EX = 3, MEM = 4, WB = 5 };

class Shape : public QGraphicsItem {
 public:
  Shape(ShapeType type = ShapeType::Block, Stage stage = Stage::IF,
        int verticalPad = 0, int horizontalPad = 0);

  QRectF boundingRect() const;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget);
  void addInput(QString input);
  void addInput(QStringList input);
  void addOutput(QString output);
  void addOutput(QStringList output);
  void setName(QString name);
  void addConnection(Connection* connection) {
    m_connections.append(connection);
  }
  void setFixedHeight(bool state, int height = 0) {
    m_isFixedHeight = state, m_fixedHeight = height;
  }

  bool isConnectedTo(Connection* connection) const;

  QPointF* getInputPoint(int index);
  QPointF* getOutputPoint(int index);
  QPointF* getTopPoint() { return &m_topPoint; }
  QPointF* getBotPoint() { return &m_bottomPoint; }

  void calculateRect();
  void calculatePoints();
  void drawTopPoint(bool state) { m_drawTopPoint = state; }
  void drawBotPoint(bool state) { m_drawBotPoint = state; }

  static int connectionType() { return QGraphicsItem::UserType + 2; }
  int type() const { return connectionType(); }

 private:
  QRectF m_rect;
  bool m_hasChanged = true;  // Flag is set whenever the item has changed (ie.
                             // when new descriptors have been added)

  QPainterPath drawALUPath(QRectF boundingRect) const;

  ShapeType m_type;
  Stage m_stage;  // Used for correctly positioning the shape
  QString m_name;

  // Extra size that is added onto each dimension of the shape.
  // Used to fine-tune the shape of the widget
  int m_verticalPad;
  int m_horizontalPad;

  QList<QString> m_inputs;
  QList<QString> m_outputs;

  QList<QPointF> m_inputPoints;
  QList<QPointF> m_outputPoints;
  bool m_drawTopPoint = false;
  bool m_drawBotPoint = false;

  QList<Connection*> m_connections;

  // Top- and bottom center-points of the shape
  // Can be used to affix labels to shapes in the graphics scene
  QPointF m_topPoint;
  QPointF m_bottomPoint;

  QFont m_nameFont;
  QFont m_ioFont;

  // drawing constants
  qreal nodeHeight = 20;
  qreal nameFontSize = 10;
  qreal ioFontSize = 8;
  qreal nodePadding = 5;  // padding between each text descriptor for a node
  qreal sidePadding =
      7;  // padding between an IO description and the side of the shape
  int m_fixedHeight;
  bool m_isFixedHeight = false;
};

}  // namespace Graphics

#endif  // SHAPE_H
