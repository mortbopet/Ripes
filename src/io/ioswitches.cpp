#include "ioswitches.h"

#include <QAbstractButton>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

namespace Ripes {

ToggleButton::ToggleButton(int trackRadius, int thumbRadius, bool rotated, QWidget* parent) : QAbstractButton(parent) {
    setCheckable(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed));
    mTrackRadius = trackRadius;
    mThumbRadius = thumbRadius;
    mAnimation = new QPropertyAnimation(this);
    mAnimation->setTargetObject(this);

    mMargin = 0 > (mThumbRadius - mTrackRadius) ? 0 : (mThumbRadius - mTrackRadius);
    mBaseOffset = mThumbRadius > mTrackRadius ? mThumbRadius : mTrackRadius;
    mEndOffset.insert(true, 4 * mTrackRadius + 2 * mMargin - mBaseOffset);  // width - offset
    mEndOffset.insert(false, mBaseOffset);
    mOffset = mBaseOffset;
    mRotated = rotated;
    QPalette palette = this->palette();

    if (mThumbRadius > mTrackRadius) {
        mTrackColor.insert(true, palette.highlight());
        mTrackColor.insert(false, palette.dark());
        mThumbColor.insert(true, palette.highlight());
        mThumbColor.insert(false, palette.light());
        mTextColor.insert(true, palette.highlightedText().color());
        mTextColor.insert(false, palette.dark().color());
        mOpacity = 0.5;
    } else {
        mTrackColor.insert(true, palette.highlight());
        mTrackColor.insert(false, palette.dark());
        mThumbColor.insert(true, palette.highlightedText());
        mThumbColor.insert(false, palette.light());
        mTextColor.insert(true, palette.highlight().color());
        mTextColor.insert(false, palette.dark().color());
        mOpacity = 1.0;
    }
}

ToggleButton::~ToggleButton() {
    delete mAnimation;
}

void ToggleButton::setChecked(bool checked) {
    QAbstractButton::setChecked(checked);
    mOffset = mEndOffset.value(checked);
}

QSize ToggleButton::sizeHint() const {
    int w = 4 * mTrackRadius + 2 * mMargin;
    int h = 2 * mTrackRadius + 2 * mMargin;

    return mRotated ? QSize(h, w) : QSize(w, h);
}

int ToggleButton::offset() {
    return mOffset;
}

void ToggleButton::setOffset(int value) {
    mOffset = value;
    update();
}

void ToggleButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QPainter::RenderHints m_paintFlags = QPainter::RenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    p.setRenderHints(m_paintFlags, true);
    p.setPen(Qt::NoPen);
    bool check = isChecked();
    qreal trackOpacity = mOpacity;
    qreal textOpacity = 1.0;
    qreal thumbOpacity = 1.0;
    QBrush trackBrush;
    QBrush thumbBrush;
    QColor textColor;

    if (this->isEnabled()) {
        trackBrush = mTrackColor[check];
        thumbBrush = mThumbColor[check];
        textColor = mTextColor[check];
    } else {
        trackOpacity *= 0.8;
        trackBrush = this->palette().shadow();
        thumbBrush = this->palette().mid();
        textColor = this->palette().shadow().color();
    }

    p.setBrush(trackBrush);
    p.setOpacity(trackOpacity);
    const qreal trackw = width() - 2 * mMargin;
    const qreal trackh = height() - 2 * mMargin;
    p.drawRoundedRect(mMargin, mMargin, trackw, trackh, mTrackRadius, mTrackRadius);

    const qreal thumbx = mOffset - mThumbRadius;
    const qreal thumby = mBaseOffset - mThumbRadius;
    p.setBrush(thumbBrush);
    p.setOpacity(thumbOpacity);
    p.drawEllipse(mRotated ? thumby : thumbx, mRotated ? thumbx : thumby, 2 * mThumbRadius, 2 * mThumbRadius);
}

void ToggleButton::resizeEvent(QResizeEvent* e) {
    QAbstractButton::resizeEvent(e);
    mOffset = mEndOffset.value(isChecked());
}

void ToggleButton::mouseReleaseEvent(QMouseEvent* e) {
    QAbstractButton::mouseReleaseEvent(e);
    if (e->button() == Qt::LeftButton) {
        mAnimation->setDuration(100);
        mAnimation->setPropertyName("mOffset");
        mAnimation->setStartValue(mOffset);
        mAnimation->setEndValue(mEndOffset[isChecked()]);
        mAnimation->start();
    }
}

void ToggleButton::enterEvent(QEvent* event) {
    setCursor(Qt::PointingHandCursor);
    QAbstractButton::enterEvent(event);
}

/**
 * IO Switches
 */

IOSwitches::IOSwitches(QWidget* parent, uint32_t startAddr) : IOBase(parent, startAddr) {
    constexpr unsigned defaultWidth = 25;

    // Parameters
    m_parameters[SWITCHES] = IOParam(SWITCHES, "# Switches", 8, true, 1, 32);

    m_switchLayout = new QGridLayout(this);
    setLayout(m_switchLayout);

    updateSwitches();
}

QString IOSwitches::description() const {
    QStringList desc;
    desc << "Each switch maps to a bit in the memory-mapped register of the peripheral.";
    desc << "switch n = bit n";

    return desc.join('\n');
}

QString IOSwitches::name() const {
    return "Switches";  // Todo: generate unique name
}

const QVariant& IOSwitches::setParameter(unsigned ID, const QVariant& value) {
    switch (ID) {
        case SWITCHES: {
        }
    }
    return value;
}

void IOSwitches::updateSwitches() {
    const unsigned nSwitches = m_parameters.at(SWITCHES).value.toInt();
    for (int i = 0; i < nSwitches; i++) {
        if (m_switches.size() <= i) {
            auto* sw = new ToggleButton(10, 8, true, this);
            auto* label = new QLabel(QString::number(i), this);
            m_switches[i] = {label, sw};
            m_switchLayout->addWidget(label, 0, i, Qt::AlignCenter);
            m_switchLayout->addWidget(sw, 1, i, Qt::AlignCenter);
        }
    }

    // Remove extra switches if # of switches was reduced
    for (int i = nSwitches; i < m_switches.size(); i++) {
        m_switches.at(i).first->deleteLater();
        m_switches.at(i).second->deleteLater();

        m_switches.erase(i);
    }

    m_regDescs = {RegDesc{"Switches", RegDesc::RW::R, nSwitches, 0}};
}

uint32_t IOSwitches::ioRead8(uint32_t offset) {
    return regRead(offset) & 0xFF;
}
uint32_t IOSwitches::ioRead16(uint32_t offset) {
    return regRead(offset) & 0xFFFF;
}
uint32_t IOSwitches::ioRead32(uint32_t offset) {
    return regRead(offset);
}
uint32_t IOSwitches::regRead(uint32_t offset) const {
    return m_values >> (offset % 4);
}

void IOSwitches::ioWrite8(uint32_t offset, uint32_t value) {
    // Read-only
    return;
}
void IOSwitches::ioWrite16(uint32_t offset, uint32_t value) {
    // Read-only
    return;
}
void IOSwitches::ioWrite32(uint32_t offset, uint32_t value) {
    // Read-only
    return;
}

}  // namespace Ripes
