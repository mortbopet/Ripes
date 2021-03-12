#pragma once

#include <QGridLayout>
#include <QLabel>
#include <QPen>
#include <QVariant>
#include <QWidget>
#include <QtCore/QPropertyAnimation>
#include <QtWidgets/QAbstractButton>

#include "iobase.h"

namespace Ripes {

/**
 * Toggle button with slider. Based on
 * https://codereview.stackexchange.com/questions/249076/implementing-toggle-button-using-qt
 */

class ToggleButton : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(int mOffset READ offset WRITE setOffset);

public:
    explicit ToggleButton(int trackRadius, int thumbRadius, bool rotated, QWidget* parent = nullptr);
    ~ToggleButton();

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void enterEvent(QEvent* event) override;
    void setChecked(bool checked);

    int offset();
    void setOffset(int value);

private:
    bool mRotated = false;
    qreal mOffset;
    qreal mBaseOffset;
    qreal mMargin;
    qreal mTrackRadius;
    qreal mThumbRadius;
    qreal mOpacity;
    QPropertyAnimation* mAnimation;

    QHash<bool, qreal> mEndOffset;
    QHash<bool, QBrush> mTrackColor;
    QHash<bool, QBrush> mThumbColor;
    QHash<bool, QColor> mTextColor;
    QHash<bool, QString> mThumbText;
};

class IOSwitches : public IOBase {
    Q_OBJECT

    enum Parameters { SWITCHES };

public:
    IOSwitches(QWidget* parent, uint32_t startAddr);
    ~IOSwitches(){};

    uint32_t startAddr() const { return m_startAddr; }
    virtual uint32_t size() const override { return 4; }
    uint32_t endAddr() const { return startAddr() + size(); }
    virtual QString description() const override;
    virtual QString name() const override;

    virtual const QVariant& setParameter(unsigned ID, const QVariant& value) override;
    virtual const std::vector<RegDesc>& registers() const override { return m_regDescs; };

    /**
     * Hardware read/write functions
     */
    virtual uint32_t ioRead8(uint32_t offset) override;
    virtual void ioWrite8(uint32_t offset, uint32_t value) override;
    virtual uint32_t ioRead16(uint32_t offset) override;
    virtual void ioWrite16(uint32_t offset, uint32_t value) override;
    virtual uint32_t ioRead32(uint32_t offset) override;
    virtual void ioWrite32(uint32_t offset, uint32_t value) override;

private:
    void updateSwitches();

    uint32_t regRead(uint32_t offset) const;
    uint32_t m_values;
    std::map<int, std::pair<QLabel*, ToggleButton*>> m_switches;
    QGridLayout* m_switchLayout;
    std::vector<RegDesc> m_regDescs;
};
}  // namespace Ripes
