#ifndef SHAPE_H
#define SHAPE_H

#include <QBrush>
#include <QFont>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QPen>

#include <set>

#include "pipelineobjects.h"

namespace Graphics {

class Connection;
enum class ShapeType { Block, ALU, MUX, Comparator, Static };
enum class Stage { IF = 1, ID = 2, EX = 3, MEM = 4, WB = 5 };
enum class SignalPos { Left, Top, Bottom };

class Shape : public QGraphicsItem {
public:
    Shape(ShapeType type = ShapeType::Block, Stage stage = Stage::IF, int verticalPad = 0, int horizontalPad = 0);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void addInput(QString input = "");
    void addInput(QStringList input);
    void addOutput(QString output = "");
    void addOutput(QStringList output);
    void setName(QString name);
    void addConnection(Connection* connection) { m_connections.append(connection); }
    void setFixedHeight(bool state, int height = 0) { m_isFixedHeight = state, m_fixedHeight = height; }

    void setSignal(SignalPos pos, SignalBase* sig);

    void setHiddenInputs(std::set<int> set) { m_hiddenInputPoints = set; }
    void setHiddenOutputs(std::set<int> set) { m_hiddenOutputPoints = set; }

    bool isConnectedTo(Connection* connection) const;

    void setSingleIOBlink(bool state) { m_singleIOBlink = state; }

    void addIOSignalPair(int pos, SignalBase* sig);

    QPointF* getInputPoint(int index);
    QPointF* getOutputPoint(int index);
    QPointF* getTopPoint() { return &m_topPoint; }
    QPointF* getBotPoint() { return &m_bottomPoint; }

    void calculateRect();
    void calculatePoints();
    void addTopPoint(QString desc = "") {
        m_drawTopPoint = true;
        m_topText = desc;
    }
    void addBotPoint(QString desc = "") {
        m_drawBotPoint = true;
        m_botText = desc;
    }
    static int connectionType() { return QGraphicsItem::UserType + 2; }
    int type() const override { return connectionType(); }

private:
    void setPointSize();

    QRectF m_rect;
    bool m_hasChanged = true;  // Flag is set whenever the item has changed (ie.
                               // when new descriptors have been added)

    QPainterPath drawALUPath(QRectF boundingRect) const;
    QString getName() const;

    const ShapeType m_type;
    Stage m_stage;  // Used for correctly positioning the shape
    QString m_name;

    // Extra size that is added onto each dimension of the shape.
    // Used to fine-tune the shape of the widget
    const int m_verticalPad;
    const int m_horizontalPad;

    QList<QString> m_inputs;
    QList<QString> m_outputs;

    QList<QPointF> m_inputPoints;
    QList<QPointF> m_outputPoints;

    // Set hidden I/O points to disable drawing of these. Used when there are unequal number of real IO points, but we
    // would like to have all I/O points drawn at the same Y-value
    std::set<int> m_hiddenInputPoints;
    std::set<int> m_hiddenOutputPoints;

    bool m_drawTopPoint = false;
    bool m_drawBotPoint = false;
    QString m_topText;
    QString m_botText;

    QList<Connection*> m_connections;

    // Top- and bottom center-points of the shape
    // Can be used to affix labels to shapes in the graphics scene
    QPointF m_topPoint;
    QPointF m_bottomPoint;

    QFont m_nameFont;
    QFont m_ioFont;

    // drawing constants
    qreal nodeHeight = 20;
    qreal nameFontSize = 14;
    qreal ioFontSize = 11;
    qreal nodePadding = 5;  // padding between each text descriptor for a node
    qreal sidePadding = 7;  // padding between an IO description and the side of the shape
    int m_fixedHeight;
    bool m_isFixedHeight = false;

    // Interfacing signals to pipeline
    SignalBase* m_leftSignal = nullptr;
    SignalBase* m_topSignal = nullptr;
    SignalBase* m_botSignal = nullptr;

    bool m_singleIOBlink = false;  // registers can be set to have individually controlled IO coloring
    std::map<int, SignalBase*> m_IOSignalPairs;
};

}  // namespace Graphics

#endif  // SHAPE_H
