#include "shape.h"
#include "pipeline.h"

#include <QPainter>

#include <QGuiApplication>
#include <QScreen>

namespace Graphics {

Shape::Shape(ShapeType type, Stage stage, int verticalPad, int horizontalPad)
    : m_verticalPad(verticalPad), m_horizontalPad(horizontalPad), m_type(type), m_stage(stage) {
    setCacheMode(DeviceCoordinateCache);

    setPointSize();
}

void Shape::setPointSize() {
    m_nameFont.setPixelSize(nameFontSize);
    m_nameFont.setBold(true);

    m_ioFont.setPixelSize(ioFontSize);
}

QRectF Shape::boundingRect() const {
    return m_rect;
}

void Shape::calculateRect() {
    // Calculates and sets the current rect of the item
    // Calculates IO points of the shape

    // Placeholder rect used for calculating (0,0) aligned text rects
    QRect largeRect = QRect(0, 0, 200, 200);

    // Get name rect
    auto fontMetricsObject = QFontMetrics(m_nameFont);
    auto nameRect = fontMetricsObject.boundingRect(largeRect, 0, m_name);

    // Get size of the largest left and right IO descriptors
    int leftIOsize = 0;
    int rightIOsize = 0;
    int leftHeight = 0;

    // Iterate over left and right IO descriptors, sum up their heights, and
    // get  the largest width of a descriptor
    fontMetricsObject = QFontMetrics(m_ioFont);
    for (const auto& input : m_inputs) {
        auto textRect = fontMetricsObject.boundingRect(largeRect, 0,
                                                       input);  // create a rect large enough to not obstruct the text
                                                                // rect
        leftHeight += textRect.height();
        int width = textRect.width();
        if (width > leftIOsize)
            leftIOsize = width;
    }
    // calculate seperate left and right IO descriptor height, and select
    // the
    // largest
    int rightHeight = 0;
    for (const auto& output : m_outputs) {
        auto textRect = fontMetricsObject.boundingRect(largeRect, 0,
                                                       output);  // create a rect large enough to not obstruct the text
                                                                 // rect
        rightHeight += textRect.height();
        int width = textRect.width();
        if (width > rightIOsize)
            rightIOsize = width;
    }

    // sum up the widths and heights, and add padding
    int width = nameRect.width() + leftIOsize + rightIOsize + m_horizontalPad + 2 * sidePadding;
    int height = leftHeight > rightHeight ? leftHeight : rightHeight;
    height += nameRect.height() + m_verticalPad;
    if (m_isFixedHeight) {
        m_rect = QRectF(-(width / 2), -(m_fixedHeight / 2), width, m_fixedHeight);
    } else {
        m_rect = QRectF(-(width / 2), -(height / 2), width, height);
    }
}

void Shape::calculatePoints() {
    // --------- CONNECTION POINTS ------------
    m_inputPoints.clear();
    m_outputPoints.clear();
    // All shapes have a top and bottom point
    m_topPoint = m_rect.topLeft();
    m_topPoint.setX(m_topPoint.x() + m_rect.width() / 2);
    m_bottomPoint = m_rect.bottomLeft();
    m_bottomPoint.setX(m_bottomPoint.x() + m_rect.width() / 2);

    if (m_type == ShapeType::ALU) {
        // ALU's are assumed to have 2 input points and 1 output point, so these
        // can be created straight away
        auto point = m_rect.topLeft();
        point.setY(m_rect.topLeft().y() + m_rect.height() / 5);
        m_inputPoints.append(point);
        point.setY(m_rect.topLeft().y() + 4 * m_rect.height() / 5);
        m_inputPoints.append(point);
        // Output point
        point = m_rect.topRight();
        point.setY(point.y() + m_rect.height() / 2);
        m_outputPoints.append(point);
    } else {
        // output points
        auto point = m_rect.topRight();
        for (int i = 1; i <= m_outputs.size(); i++) {
            point.setY(m_rect.topRight().y() + i * ((m_rect.height() / (m_outputs.size() + 1))));
            m_outputPoints.append(point);
        }

        // Input points
        point = m_rect.topLeft();
        for (int i = 1; i <= m_inputs.size(); i++) {
            point.setY(m_rect.topRight().y() + i * ((m_rect.height() / (m_inputs.size() + 1))));
            m_inputPoints.append(point);
        }
    }
}

void Shape::addInput(QString input) {
    m_inputs.append(input);
    prepareGeometryChange();
    calculateRect();
    calculatePoints();
}

void Shape::addInput(QStringList inputs) {
    for (const auto& input : inputs) {
        addInput(input);
    }
}

void Shape::addOutput(QString output) {
    m_outputs.append(output);
    prepareGeometryChange();
    calculateRect();
    calculatePoints();
}

void Shape::addOutput(QStringList outputs) {
    for (const auto& output : outputs) {
        addOutput(output);
    }
}

void Shape::setName(QString name) {
    m_name = name;
    prepareGeometryChange();
    calculateRect();
    calculatePoints();
}

bool Shape::isConnectedTo(Connection* connection) const {
    return m_connections.contains(connection);
}

QPainterPath Shape::drawALUPath(QRectF rect) const {
    QPointF topLeft = rect.topLeft();
    rect.moveTo(0, 0);
    QPainterPath path;
    path.moveTo(rect.topLeft());
    path.lineTo(rect.right(), rect.height() / 3);
    path.lineTo(rect.right(), 2 * rect.height() / 3);
    path.lineTo(rect.left(), rect.height());
    path.lineTo(rect.left(), 5 * rect.height() / 8);
    path.lineTo(rect.width() / 8, rect.height() / 2);
    path.lineTo(rect.left(), 3 * rect.height() / 8);
    path.lineTo(rect.topLeft());
    path.translate(topLeft);
    return path;
}

QString Shape::getName() const {
    if(m_type == ShapeType::Comparator){
        // Draw the logical symbol for the operation
        switch(static_cast<Pipeline::CompOp>(m_topSignal->getValue())){
            default: return QString();
            case Pipeline::CompOp::BEQ: return QString("=");
            case Pipeline::CompOp::BNE: return QString("≠");
            case Pipeline::CompOp::BLT: return QString("<");
            case Pipeline::CompOp::BLTU: return QString("<");
            case Pipeline::CompOp::BGE: return QString("≥");
            case Pipeline::CompOp::BGEU: return QString("≥");
        }
    } else {
        return m_name;
    }

}

void Shape::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
    if(!Pipeline::getPipeline()->isRunning()){
        auto rect = boundingRect();

        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

        painter->setPen(QPen(Qt::black, 1.5));
        painter->setBrush(Qt::white);
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
            case ShapeType::Comparator: {
                // Draw background color based on whether current instruction is a branch
                // operation and the outcome of the operation
                if(static_cast<bool>(m_leftSignal->getValue())) {
                    painter->setBrush(static_cast<bool>(m_botSignal->getValue()) ? Qt::green : Qt::red);
                }
                // We draw the outline of the comparator at a later stage
                painter->setPen(QPen(Qt::black, 0));
                painter->drawEllipse(rect);
                painter->setPen(QPen(Qt::black, 1.5));
                break;
            }
            case ShapeType::Static: {
                painter->drawEllipse(rect);
            } break;
        }

        painter->setBrush(Qt::transparent);

        // Translate text in relation to the bounding rectangle, and draw shape name
        auto fontMetricsObject = QFontMetrics(m_nameFont);
        QRectF textRect = fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, getName());
        textRect.moveTo(0, 0);
        textRect.translate(-textRect.width() / 2, -textRect.height() / 2);  // center text rect
        painter->setFont(m_nameFont);
        painter->drawText(textRect, Qt::AlignCenter, getName());

        // Iterate through node descriptors and draw text
        fontMetricsObject = QFontMetrics(m_ioFont);  // update fontMetrics object to new font
        painter->setFont(m_ioFont);

        // Draw input descriptors
        for (int i = 0; i < m_inputs.size(); i++) {
            textRect = fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, m_inputs[i]);
            textRect.moveTo(m_inputPoints[i]);
            textRect.translate(sidePadding, -textRect.height() / 2);  // move text away from side
            painter->drawText(textRect, m_inputs[i]);
        }

        // Draw output descriptors
        for (int i = 0; i < m_outputs.size(); i++) {
            textRect = fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, m_outputs[i]);
            textRect.moveTo(m_outputPoints[i]);
            textRect.translate(-textRect.width() - sidePadding, -textRect.height() / 2);  // move text away from side
            painter->drawText(textRect, m_outputs[i]);
        }

        // draw IO points
        if (m_drawTopPoint) {
            // draw top descriptor
            textRect = fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, m_topText);
            textRect.moveTo(m_topPoint);
            textRect.translate(-textRect.width() / 2, textRect.height());  // move text away from side
            painter->drawText(textRect, m_topText);
            if (m_topSignal != nullptr) {
                QColor col;
                if(m_type != ShapeType::Comparator){
                    col = m_topSignal->getValue() == 0 ? Qt::red : Qt::green;
                } else {
                    col = Qt::white;
                }
                // If signal has been set, draw red for off, green for on
                painter->setBrush(col);
                painter->drawEllipse(m_topPoint, 5, 5);
                painter->setBrush(Qt::transparent);
            } else {
                painter->drawEllipse(m_topPoint, 5, 5);
            }
        }

        if (m_drawBotPoint) {
            // draw bottom descriptor
            textRect = fontMetricsObject.boundingRect(QRect(0, 0, 200, 200), 0, m_botText);
            textRect.moveTo(m_bottomPoint);
            textRect.translate(-textRect.width() / 2, -textRect.height() - sidePadding);  // move text away from side
            painter->drawText(textRect, m_botText);
            if (m_botSignal != nullptr) {
                QColor col;
                if(m_type != ShapeType::Comparator){
                    col = m_botSignal->getValue() == 0 ? Qt::red : Qt::green;
                } else {
                    col = Qt::white;
                }
                painter->setBrush(col);
                painter->drawEllipse(m_bottomPoint, 5, 5);
                painter->setBrush(Qt::transparent);
            } else {
                painter->drawEllipse(m_bottomPoint, 5, 5);
            }
        }

        // LEFT/RIGHT IO POINTS
        for (uint32_t i = 0; i < m_inputPoints.length(); i++) {
            if (m_hiddenInputPoints.find(i) == m_hiddenInputPoints.end()) {
                if (m_singleIOBlink) {
                    // Color individual points
                    // find point if in ioSignalPairs
                    if (m_IOSignalPairs.find(i) != m_IOSignalPairs.end()) {
                        QColor col = m_IOSignalPairs[i]->getValue() == 0 ? Qt::red : Qt::green;
                        painter->setBrush(col);
                        painter->drawEllipse(m_inputPoints[i], 5, 5);
                        painter->setBrush(Qt::transparent);
                    } else {
                        painter->drawEllipse(m_inputPoints[i], 5, 5);
                    }

                } else if (m_leftSignal != nullptr && m_leftSignal->getValue() == i) {
                    // For multiplexers, we want to color the input that is asserted
                    painter->setBrush(Qt::red);
                    painter->drawEllipse(m_inputPoints[i], 5, 5);
                    painter->setBrush(Qt::transparent);
                } else {
                    painter->drawEllipse(m_inputPoints[i], 5, 5);
                }
            }
        }
        for (int i = 0; i < m_outputPoints.length(); i++) {
            if (m_hiddenOutputPoints.find(i) == m_hiddenOutputPoints.end())
                painter->drawEllipse(m_outputPoints[i], 5, 5);
        }

        // Hacky - to draw the outline of the comparator correctly, we redraw its ellipse
        if(m_type == ShapeType::Comparator){
            painter->setBrush(Qt::transparent);
            painter->drawEllipse(rect);
        }
    }

}

void Shape::setSignal(SignalPos pos, SignalBase* sig) {
    switch (pos) {
        case SignalPos::Left:
            m_leftSignal = sig;
            break;
        case SignalPos::Top:
            m_topSignal = sig;
            break;
        case SignalPos::Bottom:
            m_botSignal = sig;
            break;
        default:
            break;
    }
}

void Shape::addIOSignalPair(int pos, SignalBase* sig) {
    Q_ASSERT(pos < m_inputPoints.size());
    m_IOSignalPairs[pos] = sig;
}

QPointF* Shape::getInputPoint(int index) {
    Q_ASSERT(index < m_inputPoints.size());
    return &m_inputPoints[index];
}

QPointF* Shape::getOutputPoint(int index) {
    Q_ASSERT(index < m_outputPoints.size());
    return &m_outputPoints[index];
}

}  // namespace Graphics
