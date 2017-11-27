#ifndef CONNECTION_H
#define CONNECTION_H

#include <QGraphicsItem>
#include <QObject>

#include "pipelinewidget.h"

namespace Graphics {
class Shape;

enum class ValueDrawPos { Source, Middle, Destination };

// Class for drawing labels for connections. Disattached from connection, such that tooltips for connections are only
// shown when hovering over the label position
class Label : public QGraphicsItem {
    friend class Connection;

public:
    Label() {}
    void setText(QString text) { m_text = text; }

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QPointF* m_drawPos;
    Shape*   m_source;
    bool     m_showValue = false;
    QString  m_text;
};

/*
    Class for drawing connections and signal values between shapes in the
   pipeline view.
*/
class Connection : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Connection(Shape* source, QPointF* sourcePoint, Shape* dest, QPointF* destPoint);
    Connection(Shape* source, QPointF* sourcePoint, QList<QPair<Shape*, QPointF*>> dests);
    void setValueDrawPos(ValueDrawPos pos) { m_valuePos = pos, update(); }

    void setValue(uint32_t value);
    QPair<QPointF, QPointF> getPoints() const;
    QPair<Shape*, Shape*>   getShapes() const { return QPair<Shape*, Shape*>(m_source, m_dests[0].first); }
    Shape* getSource() { return m_source; }

    static int connectionType() { return QGraphicsItem::UserType + 1; }
    int        type() const { return connectionType(); }
    void setKinkBiases(QList<int> biases) { m_kinkBiases = biases; }
    void setKinkBias(int bias) { m_kinkBiases = QList<int>() << bias; }
    void setKinkPoints(QList<int> points) { m_kinkPoints = points; }
    void setFeedbackSettings(bool dir, int sourceStubLen, int destStubLen) {
        m_feedbackDir   = dir ? 1 : -1;
        m_sourceStubLen = sourceStubLen;
        m_destStubLen   = destStubLen;
    }
    void addLabelToScene();

public slots:
    void showValue(bool state) { m_label.m_showValue = state; }

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    // A list of IO points, which all connect to this widget -
    // used to form attachment points to this connection
    QList<QPointF*> m_connectsToThis;

    // List of biases for each kink in the connection
    // Used to tune kink positioning
    QList<int> m_kinkBiases;
    QList<int> m_kinkPoints;
    int        m_feedbackDir = 1;  // 1: down, -1: up
    int        m_sourceStubLen;
    int        m_destStubLen;

    // Variables related to the current value of the connection (signal value)
    ValueDrawPos m_valuePos = ValueDrawPos::Source;

    int m_arrowSize = 10;

    Shape*   m_source;
    QPointF* m_sourcePointPtr;
    QList<QPair<Shape*,QPointF*>>   m_dests;
    Label    m_label;
};

}  // namespace Graphics

#endif  // CONNECTION_H
