#include "shape.h"

#include <QPainter>

namespace Graphics {

Shape::Shape(ShapeType type, int verticalPad, int horizontalPad)
    : m_verticalPad(verticalPad), m_horizontalPad(horizontalPad), m_type(type) {
    setCacheMode(DeviceCoordinateCache);
}

QRectF Shape::boundingRect() const {
    if (m_type == ShapeType::Block) {
        return calculateRect();
    }
    return QRectF();
}
void Shape::addInput(QString input) { m_inputs.append(input); }
void Shape::addInput(QStringList inputs) {
    for (const auto &input : inputs) {
        addInput(input);
    }
}

QRectF Shape::calculateRect() const {
    auto fontMetricsObject = QFontMetrics(QFont());
    auto nameRect = fontMetricsObject.boundingRect(m_name);

    // Get size of the largest left and right IO descriptors
    int leftIOsize = 0;
    int rightIOsize = 0;
    int leftHeight = 0;

    // Iterate over left and right IO descriptors, sum up their heights, and get
    // the largest width of a descriptor
    for (const auto &input : m_inputs) {
        auto textRect = fontMetricsObject.boundingRect(
          QRect(0, 0, 200, 200), 0,
          input);  // create a rect large enough to not obstruct the text rect
        leftHeight += textRect.height();
        int width = textRect.width();
        if (width > leftIOsize) leftIOsize = width;
    }
    // calculate seperate left and right IO descriptor height, and select the
    // largest
    int rightHeight = 0;
    for (const auto &output : m_outputs) {
        QRect textRect = fontMetricsObject.boundingRect(
          QRect(0, 0, 200, 200), 0,
          output);  // create a rect large enough to not obstruct the text rect
        rightHeight += textRect.height();
        int width = textRect.width();
        if (width > rightIOsize) rightIOsize = width;
    }

    // sum up the widths and heights, and add padding
    int width = nameRect.width() + leftIOsize + rightIOsize + m_horizontalPad +
                2 * sidePadding;
    int height = leftHeight > rightHeight ? leftHeight : rightHeight;
    height += nameRect.height() + m_verticalPad;

    return QRectF(-(width / 2), -(height / 2), width, height);
}

void Shape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *) {
    auto boundingRect = calculateRect();

    switch (m_type) {
        case ShapeType::Block: {
            painter->setPen(QPen(Qt::black, 1));
            painter->drawRect(boundingRect);
        } break;
        case ShapeType::ALU: {
        } break;
        case ShapeType::MUX: {
        } break;
    }

    // Translate text in relation to the bounding rectangle, and draw text
    QFont font;
    font.setPointSize(nameFontSize);
    font.setBold(true);
    auto fontMetricsObject = QFontMetrics(font);
    QRectF textRect =
      fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, m_name);
    textRect.moveTo(0, 0);
    textRect.translate(-textRect.width() / 2,
                       -textRect.height() / 2);  // center text rect
    painter->setFont(font);
    painter->drawText(textRect, m_name);

    // Iterate through node descriptors and draw text
    font.setPointSize(ioFontSize);
    font.setBold(false);
    fontMetricsObject =
      QFontMetrics(font);  // update fontMetrics object to new font
    painter->setFont(font);

    // Inputs
    QPointF top = boundingRect.topLeft();
    for (const auto &input : m_inputs) {
        textRect =
          fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, input);
        textRect.moveTo(top);
        textRect.translate(sidePadding, 0);  // move text away from side
        painter->drawText(textRect, input);
        top.setY(top.y() + textRect.height() + nodePadding);
    }

    // Outputs
    top = boundingRect.topRight();
    for (const auto &output : m_outputs) {
        textRect =
          fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, output);
        textRect.moveTo(top);
        textRect.translate(-textRect.width(), 0);
        textRect.translate(-sidePadding, 0);  // Move text away from side
        painter->drawText(textRect, output);
        top.setY(top.y() + textRect.height() + nodePadding);
    }
}

}  // namespace Graphics
