#include "ioledmatrix.h"

#include <QPainter>
#include <QPen>

namespace Ripes {

IOLedMatrix::IOLedMatrix(QWidget* parent, uint32_t startAddr, unsigned maxWidth)
    : IOBase(parent, startAddr), m_maxWidth(maxWidth) {
    constexpr unsigned defaultWidth = 10;

    // Parameters
    m_parameters[WIDTH] = IOParam(WIDTH, "Matrix width", defaultWidth, true, 0, m_maxWidth);
    m_parameters[SIZE] = IOParam(SIZE, "LED size", 25, true, 1, 100);

    m_pen.setWidth(1);
    m_pen.setColor(Qt::black);

    updateLEDRegs();
}

const QVariant& IOLedMatrix::setParameter(unsigned ID, const QVariant& value) {
    return QVariant();
}

uint32_t IOLedMatrix::ioRead8(uint32_t offset) {
    return regRead(offset) & 0xFF;
}
uint32_t IOLedMatrix::ioRead16(uint32_t offset) {
    return regRead(offset) & 0xFFFF;
}
uint32_t IOLedMatrix::ioRead32(uint32_t offset) {
    return regRead(offset);
}
uint32_t IOLedMatrix::regRead(uint32_t offset) const {
    return m_ledRegs.at(offset / 4) >> (offset % 4);
}

void IOLedMatrix::ioWrite8(uint32_t offset, uint32_t value) {}
void IOLedMatrix::ioWrite16(uint32_t offset, uint32_t value) {}
void IOLedMatrix::ioWrite32(uint32_t offset, uint32_t value) {}

inline QColor regToColor(uint32_t regVal) {
    return QColor(regVal >> 16 & 0xFF, regVal >> 8 & 0xFF, regVal & 0xFF);
}

void IOLedMatrix::updateLEDRegs() {
    const int width = m_parameters[WIDTH].value.toInt();
    m_ledRegs.resize(width * width);

    // DEBUG: test pattern
    for (int y = 0; y < width; y++) {
        for (int x = 0; x < width; x++) {
            const int r = (0xFF * x) / width;
            const int g = (0xFF * y) / width;
            const int b = (0xFF * (x + y)) / (width + width);

            m_ledRegs.at(y * width + x) = r << 16 | g << 8 | b;
        }
    }
}

QSize IOLedMatrix::minimumSizeHint() const {
    const int width = m_parameters.at(WIDTH).value.toInt();
    const int size = m_parameters.at(SIZE).value.toInt();
    const int pixelWidth = width * (size + 2);
    return QSize(pixelWidth, pixelWidth);
}

void IOLedMatrix::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    painter.setPen(m_pen);

    const int width = m_parameters[WIDTH].value.toInt();
    const int size = m_parameters[SIZE].value.toInt();
    for (int y = 0; y < width; y++) {
        for (int x = 0; x < width; x++) {
            QBrush brush(regToColor(m_ledRegs.at(y * width + x)));
            painter.setBrush(brush);

            const unsigned xpos = x * size + size / 2;
            const unsigned ypos = y * size + size / 2;

            painter.drawEllipse(xpos, ypos, size, size);
        }
    }

    painter.end();
}

}  // namespace Ripes
