#pragma once

#include <QPen>
#include <QVariant>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QAbstractButton);

#include "iobase.h"

namespace Ripes {

class IODPad : public IOBase {
    Q_OBJECT

    enum IdxToDir { UP, DOWN, LEFT, RIGHT, DIRECTIONS };

public:
    IODPad(QWidget* parent);
    ~IODPad() { unregister(); };

    virtual uint32_t size() const override;
    virtual QString description() const override;
    virtual QString baseName() const override { return "D-Pad"; };

    virtual const std::vector<RegDesc>& registers() const override { return m_regDescs; };

    /**
     * Hardware read/write functions
     */
    virtual uint32_t ioRead(uint32_t offset, unsigned size) override;
    virtual void ioWrite(uint32_t offset, uint32_t value, unsigned size) override;

protected:
    virtual void parameterChanged(unsigned) override{/* no parameters */};
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

private:
    unsigned m_maxSideWidth = 256;
    std::vector<uint32_t> m_ledRegs;
    std::vector<RegDesc> m_regDescs;
    std::map<IdxToDir, QAbstractButton*> m_buttons;
};
}  // namespace Ripes
