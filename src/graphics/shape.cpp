#include "shape.h"

#include <QPainter>

namespace Graphics {

Shape::Shape(ShapeType type, int verticalPad, int horizontalPad)
    : m_verticalPad(verticalPad), m_horizontalPad(horizontalPad), m_type(type) {
    setCacheMode(DeviceCoordinateCache);
    calculateRect();

    auto rect = boundingRect();

    // ALU's are assumed to have 2 input points and 1 output point, so these
    // can be created straight away
    if (m_type == ShapeType::ALU) {
        auto point = rect.topLeft();
        point.setY(rect.topLeft().y() + rect.height() / 5);
        m_inputPoints.append(point);
        point.setY(rect.topLeft().y() + 4 * rect.height() / 5);
        m_inputPoints.append(point);
        // Output point
        point = rect.topRight();
        point.setY(point.y() + rect.height() / 2);
        m_outputPoints.append(point);
    }
}

QRectF Shape::boundingRect() const { return m_rect; }

void Shape::calculateRect() {
    // Calculates and sets the current rect of the item

    // Placeholder rect used for calculating (0,0) aligned text rects
    QRect largeRect = QRect(0, 0, 200, 200);
    auto fontMetricsObject = QFontMetrics(QFont());
    auto nameRect = fontMetricsObject.boundingRect(largeRect, 0, m_name);

    // Get size of the largest left and right IO descriptors
    int leftIOsize = 0;
    int rightIOsize = 0;
    int leftHeight = 0;

    // Iterate over left and right IO descriptors, sum up their heights, and
    // get
    // the largest width of a descriptor
    for (const auto &input : m_inputs) {
        auto textRect = fontMetricsObject.boundingRect(
          largeRect, 0,
          input);  // create a rect large enough to not obstruct the text
                   // rect
        leftHeight += textRect.height();
        int width = textRect.width();
        if (width > leftIOsize) leftIOsize = width;
    }
    // calculate seperate left and right IO descriptor height, and select
    // the
    // largest
    int rightHeight = 0;
    for (const auto &output : m_outputs) {
        QRect textRect = fontMetricsObject.boundingRect(
          largeRect, 0,
          output);  // create a rect large enough to not obstruct the text
                    // rect
        rightHeight += textRect.height();
        int width = textRect.width();
        if (width > rightIOsize) rightIOsize = width;
    }

    // sum up the widths and heights, and add padding
    int width = nameRect.width() + leftIOsize + rightIOsize + m_horizontalPad +
                2 * sidePadding;
    int height = leftHeight > rightHeight ? leftHeight : rightHeight;
    height += nameRect.height() + m_verticalPad;
    m_rect = QRectF(-(width / 2), -(height / 2), width, height);
}

void Shape::addInput(QString input) {
    m_inputs.append(input);
    prepareGeometryChange();
    calculateRect();
}

void Shape::addInput(QStringList inputs) {
    for (const auto &input : inputs) {
        addInput(input);
    }
}

void Shape::addOutput(QString output) {
    m_outputs.append(output);
    prepareGeometryChange();
    calculateRect();
}

void Shape::addOutput(QStringList outputs) {
    for (const auto &output : outputs) {
        addOutput(output);
    }
}

void Shape::setName(QString name) {
    m_name = name;
    prepareGeometryChange();
    calculateRect();
}

QPainterPath Shape::drawALUPath(QRectF rect) const {
    QPointF topLeft = rect.topLeft();
    rect.moveTo(0, 0);
    QPainterPath path;
    path.moveTo(rect.topLeft());
    path.lineTo(rect.right(), rect.height() / 4);
    path.lineTo(rect.right(), 3 * rect.height() / 4);
    path.lineTo(rect.left(), rect.height());
    path.lineTo(rect.left(), 5 * rect.height() / 8);
    path.lineTo(rect.width() / 8, rect.height() / 2);
    path.lineTo(rect.left(), 3 * rect.height() / 8);
    path.lineTo(rect.topLeft());
    path.translate(topLeft);
    return path;
}

void Shape::paint(QPainter *painter,
                  const QStyleOptionGraphicsItem * /*option*/, QWidget *) {
    auto rect = boundingRect();

    painter->setPen(QPen(Qt::black, 1));
    switch (m_type) {
        case ShapeType::Block: {
            painter->drawRect(rect);
        } break;
        case ShapeType::ALU: {
            painter->drawPath(drawALUPath(rect));
        } break;
        case ShapeType::MUX: {
            painter->drawRoundedRect(rect, 40, 15);
        } break;
    }

    // Translate text in relation to the bounding rectangle, and draw shape name
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
    if (m_type == ShapeType::ALU) {
        // Shift ALU name a bit to the right
        textRect.translate(-rect.width() / 8, 0);
    }
    painter->drawText(textRect, Qt::AlignCenter, m_name);

    // Iterate through node descriptors and draw text
    font.setPointSize(ioFontSize);
    font.setBold(false);
    fontMetricsObject =
      QFontMetrics(font);  // update fontMetrics object to new font
    painter->setFont(font);

    // Draw input descriptors
    QPointF top = rect.topLeft();
    for (const auto &input : m_inputs) {
        textRect =
          fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, input);
        textRect.moveTo(top);
        textRect.translate(sidePadding, 0);  // move text away from side
        painter->drawText(textRect, input);
        top.setY(top.y() + textRect.height() + nodePadding);
    }

    // Draw output descriptors
    top = rect.topRight();
    for (const auto &output : m_outputs) {
        textRect =
          fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, output);
        textRect.moveTo(top);
        textRect.translate(-textRect.width(), 0);
        textRect.translate(-sidePadding, 0);  // Move text away from side
        if (m_type == ShapeType::ALU) {
            // With ALU's we assume óne output descriptor, and move it to the
            // center of the ALU
            textRect.translate(0, rect.height() / 2 - textRect.height() / 2);
        }
        painter->drawText(textRect, output);
        top.setY(top.y() + textRect.height() + nodePadding);
    }
}

}  // namespace Graphics
