#include "ioledmatrix.h"

#include <QPainter>
#include <QPen>

namespace Ripes {

IOLedMatrix::IOLedMatrix(QWidget* parent) : IOBase(parent) {
    constexpr unsigned defaultWidth = 25;

    // Parameters
    m_parameters[HEIGHT] = IOParam(WIDTH, "Height", defaultWidth, true, 0, m_maxSideWidth);
    m_parameters[WIDTH] = IOParam(WIDTH, "Width", defaultWidth + 10, true, 0, m_maxSideWidth);
    m_parameters[SIZE] = IOParam(SIZE, "LED size", 8, true, 1, 100);

    m_pen.setWidth(1);
    m_pen.setColor(Qt::black);

    updateLEDRegs();
}

uint32_t IOLedMatrix::size() const {
    const int width = m_parameters.at(WIDTH).value.toInt();
    const int height = m_parameters.at(HEIGHT).value.toInt();
    const int nLEDs = width * height;

    return nLEDs * 4;
}

QString IOLedMatrix::description() const {
    QStringList desc;
    desc << "Each LED maps to a 24-bit register storing an RGB color value, with B stored in the least significant "
            "byte.";
    desc << "The byte offset of the LED at coordinates (x, y) is:";
    desc << "    offset = (y + x*N_LEDS_ROW) * 4";

    return desc.join('\n');
}

QString IOLedMatrix::name() const {
    return "LED Matrix";  // Todo: generate unique name
}

uint32_t IOLedMatrix::ioRead(uint32_t offset, unsigned size) {
    return (m_ledRegs.at(offset / 4) >> (offset % 4)) & generateBitmask(size * 8);
}

void IOLedMatrix::ioWrite(uint32_t offset, uint32_t value, unsigned size) {
    offset >>= 2;  // word addressable
    if (offset >= m_ledRegs.size()) {
        Q_ASSERT(false);
    }
    m_ledRegs.at(offset) = value;
    emit scheduleUpdate();
}

inline QColor regToColor(uint32_t regVal) {
    return QColor(regVal >> 16 & 0xFF, regVal >> 8 & 0xFF, regVal & 0xFF);
}

void IOLedMatrix::updateLEDRegs() {
    const int width = m_parameters[WIDTH].value.toInt();
    const int height = m_parameters[HEIGHT].value.toInt();
    const int nLEDs = width * height;
    m_ledRegs.resize(nLEDs);

    m_regDescs.clear();
    m_regDescs.resize(nLEDs);
    for (unsigned i = 0; i < m_regDescs.size(); i++) {
        m_regDescs.at(i) = RegDesc{"LED_" + QString::number(i), RegDesc::RW::RW, 24, i * 4};
    }

    updateGeometry();
    resize(minimumSize());
    emit regMapChanged();
}

QSize IOLedMatrix::minimumSizeHint() const {
    const int width = m_parameters.at(WIDTH).value.toInt();
    const int height = m_parameters.at(HEIGHT).value.toInt();
    const int size = m_parameters.at(SIZE).value.toInt();
    const int pixelWidth = width * (size + m_pen.width());
    const int pixelHeight = height * (size + m_pen.width());
    return QSize(pixelWidth, pixelHeight);
}

void IOLedMatrix::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(m_pen);

    const int width = m_parameters[WIDTH].value.toInt();
    const int height = m_parameters[HEIGHT].value.toInt();
    const int size = m_parameters[SIZE].value.toInt();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QBrush brush(regToColor(m_ledRegs.at(y * width + x)));
            painter.setBrush(brush);

            const unsigned xpos = x * (size + m_pen.width());
            const unsigned ypos = y * (size + m_pen.width());

            painter.drawEllipse(xpos, ypos, size, size);
        }
    }

    painter.end();
}

}  // namespace Ripes
