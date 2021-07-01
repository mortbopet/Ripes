#include "iodpad.h"
#include "ioregistry.h"

#include <QGridLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QPen>
#include <QToolButton>

namespace Ripes {

IODPad::IODPad(QWidget* parent) : IOBase(IOType::DPAD, parent) {
    for (unsigned i = 0; i < DIRECTIONS; ++i) {
        QString name;
        Qt::ArrowType arrow;
        switch (i) {
            case UP:
                name = "UP";
                arrow = Qt::UpArrow;
                break;
            case DOWN:
                name = "DOWN";
                arrow = Qt::DownArrow;
                break;
            case LEFT:
                name = "LEFT";
                arrow = Qt::LeftArrow;
                break;
            case RIGHT:
                name = "RIGHT";
                arrow = Qt::RightArrow;
                break;
        }
        auto* button = new QToolButton();
        m_buttons[static_cast<IdxToDir>(i)] = button;
        button->setArrowType(arrow);

        m_regDescs.push_back(RegDesc{name, RegDesc::RW::R, 1, i * 4, true});
    }

    auto* gridLayout = new QGridLayout();
    gridLayout->addWidget(m_buttons[UP], 0, 1);
    gridLayout->addWidget(m_buttons[DOWN], 2, 1);
    gridLayout->addWidget(m_buttons[LEFT], 1, 0);
    gridLayout->addWidget(m_buttons[RIGHT], 1, 2);

    setLayout(gridLayout);
}

unsigned IODPad::byteSize() const {
    return 4 * 4;
}

void IODPad::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
        case Qt::Key_A:
            m_buttons.at(LEFT)->setDown(true);
            return;
        case Qt::Key_D:
            m_buttons.at(RIGHT)->setDown(true);
            return;
        case Qt::Key_W:
            m_buttons.at(UP)->setDown(true);
            return;
        case Qt::Key_S:
            m_buttons.at(DOWN)->setDown(true);
            return;
    }
    IOBase::keyPressEvent(e);
}
void IODPad::keyReleaseEvent(QKeyEvent* e) {
    switch (e->key()) {
        case Qt::Key_A:
            m_buttons.at(LEFT)->setDown(false);
            return;
        case Qt::Key_D:
            m_buttons.at(RIGHT)->setDown(false);
            return;
        case Qt::Key_W:
            m_buttons.at(UP)->setDown(false);
            return;
        case Qt::Key_S:
            m_buttons.at(DOWN)->setDown(false);
            return;
    }
    IOBase::keyReleaseEvent(e);
}

QString IODPad::description() const {
    QStringList desc;
    desc << "Each button maps to a 32-bit register, with the least-significant bit indicating the state of the "
            "button.\n";
    desc << "If the D-pad window is in focus, the buttons may be pressed using the \"WASD\" keys of the keyboard.";

    return desc.join('\n');
}

VInt IODPad::ioRead(AInt offset, unsigned) {
    switch (offset) {
        case LEFT * 4: {
            return m_buttons.at(LEFT)->isDown();
        }
        case RIGHT * 4: {
            return m_buttons.at(RIGHT)->isDown();
        }
        case UP * 4: {
            return m_buttons.at(UP)->isDown();
        }
        case DOWN * 4: {
            return m_buttons.at(DOWN)->isDown();
        }
    }
    return 0;
}

void IODPad::ioWrite(AInt, VInt, unsigned) {
    // Write-only
}

}  // namespace Ripes
