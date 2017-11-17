#include "shape.h"

#include <QPainter>

namespace Graphics {

Shape::Shape() { setCacheMode(DeviceCoordinateCache); }

QRectF Shape::boundingRect() const {
    if (m_type == ShapeType::Block) {
        return calculateRect();
    }
    return QRectF();
}

QRectF Shape::calculateRect() const {
    auto fontMetricsObject = QFontMetrics(QFont());
    auto nameRect = fontMetricsObject.boundingRect(m_name);

    // Base height on largest number of IO nodes
    int height = m_inputs.size() * nodeHeight;
    height = (m_outputs.size() * nodeHeight > height)
               ? m_outputs.size() * nodeHeight
               : height;
    height += 10 + nameRect.height();  // add padding and height of the name

    // Get size of the largest left and right IO descriptors
    int leftIOsize = 0;
    int rightIOsize = 0;

    for (const auto &input : m_inputs) {
        int width = fontMetricsObject.boundingRect(input).width();
        if (width > leftIOsize) leftIOsize = width;
    }
    for (const auto &output : m_outputs) {
        int width = fontMetricsObject.boundingRect(output).width();
        if (width > rightIOsize) rightIOsize = width;
    }

    int width = nameRect.width() + leftIOsize + rightIOsize +
                10;  // sum up text widths and add some padding

    return QRectF(-(width / 2), -(height / 2), width, height);
}

void Shape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget) {
    switch (m_type) {
        case ShapeType::Block: {
            painter->setPen(QPen(Qt::black, 2));
            painter->drawRect(calculateRect());
        } break;
        case ShapeType::ALU: {
        } break;
        case ShapeType::MUX: {
        } break;
    }
}

}  // namespace Graphics
