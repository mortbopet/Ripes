#pragma once

#include <QPen>
#include <QVariant>
#include <QWidget>

#include "iobase.h"

namespace Ripes {

class IOLedMatrix : public IOBase {
    Q_OBJECT

    enum Parameters { HEIGHT, WIDTH, SIZE };

public:
    IOLedMatrix(QWidget* parent);
    ~IOLedMatrix(){};

    virtual uint32_t size() const override { return (m_maxSideWidth * m_maxSideWidth) * 4; }
    virtual QString description() const override;
    virtual QString name() const override;

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

protected:
    virtual void parameterChanged(unsigned) override { updateLEDRegs(); };

    /**
     * QWidget drawing
     */
    void paintEvent(QPaintEvent* event) override;
    QSize minimumSizeHint() const override;

private:
    uint32_t regRead(uint32_t offset) const;
    void updateLEDRegs();

    unsigned m_maxSideWidth = 256;
    std::vector<uint32_t> m_ledRegs;
    std::vector<RegDesc> m_regDescs;

    QPen m_pen;
};
}  // namespace Ripes
