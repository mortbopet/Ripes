#ifndef BACKGROUNDITEMS_H
#define BACKGROUNDITEMS_H

#include <QFont>
#include <QGraphicsItem>

namespace Graphics {
class Shape;

class DashLine : public QGraphicsItem {
public:
    DashLine(Shape* reg);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Shape* m_regPtr;
    constexpr static int dashHeight = 100;
};

class Text : public QGraphicsItem {
public:
    Text(QPointF pos);
    void setText(const QString& text, QColor col = QColor());
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QString m_text;
    QFont m_font;
    QPointF m_pos;
    QColor m_textColor;
};

}  // namespace Graphics

#endif  // BACKGROUNDITEMS_H
