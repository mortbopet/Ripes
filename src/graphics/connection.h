#ifndef CONNECTION_H
#define CONNECTION_H

#include <QGraphicsItem>
#include <QObject>

#include <algorithm>

#include "pipelineobjects.h"
#include "pipelinewidget.h"

namespace Graphics {
class Shape;

typedef QPair<Shape*, QPointF*> PointPair;

enum class ValueDrawPos { Source, Middle, Destination };

// Class for drawing labels for connections. Disattached from connection, such that tooltips for connections are only
// shown when hovering over the label position
class Label : public QGraphicsItem {
    friend class Connection;

public:
    Label() {}

protected:
    QRectF getTextRect(const QString& text) const;
    QString getText() const;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    void setSignal(SignalBase* signal) { m_signal = signal; }
    QPointF* m_drawPos;
    Shape* m_source;
    SignalBase* m_signal = nullptr;
};

/*
    Class for drawing connections and signal values between shapes in the
   pipeline view.
*/
enum class Direction { east, west, north, south };
class Connection : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Connection(Shape* source, QPointF* sourcePoint, Shape* dest, QPointF* destPoint);
    Connection(Shape* source, QPointF* sourcePoint, QList<PointPair> dests);
    void setValueDrawPos(ValueDrawPos pos) { m_valuePos = pos, update(); }
    void drawPointAtFirstKink(bool val) { m_pointAtFirstKink = val; }

    void finalize();
    void updateLabel();
    QPair<QPointF, QPointF> getPoints() const;
    QPair<Shape*, Shape*> getShapes() const { return QPair<Shape*, Shape*>(m_source, m_dests[0].first); }
    Shape* getSource() { return m_source; }
    QGraphicsItem* getLabel() { return &m_label; }

    static int connectionType() { return QGraphicsItem::UserType + 1; }
    int type() const override { return connectionType(); }
    void setKinkBiases(QList<int> biases) { m_kinkBiases = biases; }
    Connection* setKinkBias(int bias) {
        m_kinkBiases = QList<int>() << bias;
        return this;
    }
    void setKinkPoints(QList<int> points) { m_kinkPoints = points; }
    void setFeedbackSettings(bool dir, int sourceStubLen, int destStubLen) {
        m_feedbackDir = dir ? 1 : -1;
        m_sourceStubLen = sourceStubLen;
        m_destStubLen = destStubLen;
    }
    void addInvalidDestSourcePoint(int i) { m_invalidDestSourcePoints.append(i); }
    void addLabelToScene();
    Connection* setDirection(Direction rot) {
        m_dir = rot;
        return this;
    }

    void setSignal(SignalBase* sig) { m_label.setSignal(sig); }

public slots:
    void toggleLabel(bool show);

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QRectF calculateBoundingRect() const;
    void calculatePaths();

    // A list of IO points, which all connect to this widget -
    // used to form attachment points to this connection
    QList<QPointF*> m_connectsToThis;
    QRectF m_boundingRect;

    // List of biases for each kink in the connection
    // Used to tune kink positioning
    QList<int> m_kinkBiases;
    QList<int> m_kinkPoints;
    int m_feedbackDir = 1;  // 1: down, -1: up
    int m_sourceStubLen;
    int m_destStubLen;

    Direction m_dir = Direction::east;  // By default, all arrows point towards east

    // Variables related to the current value of the connection (signal value)
    ValueDrawPos m_valuePos = ValueDrawPos::Source;

    int m_arrowSize = 10;
    QList<QPolygonF> m_polyLines;
    Shape* m_source;
    QPointF* m_sourcePointPtr;
    QList<PointPair> m_dests;
    QList<int> m_invalidDestSourcePoints;  // used when algorithm shouldnt use created source points for a destination,
                                           // when calculating connections to other destinations
    Label m_label;

    bool m_pointAtFirstKink = false;
};

}  // namespace Graphics

// define < operator for QPointF to be able to use it in a map
inline bool operator<(const QPointF& a, const QPointF& b) {
    return a.manhattanLength() < b.manhattanLength();
}

#endif  // CONNECTION_H
