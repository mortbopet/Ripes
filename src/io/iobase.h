#pragma once

#include <QVariant>
#include <QWidget>
#include <typeindex>
#include <typeinfo>

#include "binutils.h"

namespace Ripes {

struct IOParam {
    IOParam(unsigned _id, QString _name, QVariant _defValue, bool _hasRange, QVariant _min = QVariant(),
            QVariant _max = QVariant())
        : id(_id), name(_name), value(_defValue), defaultValue(_defValue), hasRange(_hasRange), min(_min), max(_max) {}

    IOParam() {}

    unsigned id = -1;
    QString name;
    QVariant value;
    QVariant defaultValue;
    bool hasRange = false;
    QVariant min;
    QVariant max;
};

struct RegDesc {
    enum class RW { R, W, RW };
    QString name;
    RW rw;
    unsigned bitWidth;
    uint32_t address;
};

class IOBase : public QWidget {
    Q_OBJECT

public:
    IOBase(QWidget* parent);
    virtual ~IOBase() {
        assert(m_didUnregister && "IO peripherals must call unregister() in their destructor!");
        decrementPeriphCounter(this);
    };

    const std::map<unsigned, IOParam>& parameters() const { return m_parameters; }
    virtual QString description() const = 0;
    virtual QString name() const = 0;

    /**
     * @brief registers
     * @return a description of the programmable interface of this peripheral
     */
    virtual const std::vector<RegDesc>& registers() const = 0;

    /**
     * @brief setParameter
     * Attempt to set the parameter @p ID to @p value. Returns true if the value was set successfully.
     */
    virtual bool setParameter(unsigned ID, const QVariant& value);
    virtual uint32_t size() const = 0;

    /**
     * Hardware read/write functions
     */
    virtual uint32_t ioRead(uint32_t offset, unsigned size) = 0;
    virtual void ioWrite(uint32_t offset, uint32_t value, unsigned size) = 0;

signals:
    /**
     * @brief regMapChanged
     * Should be emitted every time the register map of this peripheral changes.
     */
    void regMapChanged();

    /**
     * @brief sizeChanged
     * Should be emitted every time the size of this peripheral changes.
     */
    void sizeChanged();

    /**
     * @brief scheduleUpdate
     * Should be emitted when a peripheral requests to be repainted. We do this through signal/slot mechanisms to ensure
     * that the actual update() is only performed on the GUI thread.
     */
    void scheduleUpdate();

    /**
     * @brief aboutToDelete
     * Signal emitted when this IO peripheral is about to be destroyed. @param ok should be set to 'true' once the
     * surrounding environment acknowledges (ie. after peripheral-dependees have been notified).
     */
    void aboutToDelete(std::atomic<bool>& ok);

protected:
    /**
     * @brief unregister
     * Unregisters the IO peripheral with the remaining compute system in a thread-safe manner. This function is key in
     * avoiding race conditions from a processor model executing on another thread, trying to access this peripherals
     * i/o functions while it is being deleted. This function !must! be called from an inheriting IO peripherals'
     * constructor.
     */
    void unregister();

    virtual void parameterChanged(unsigned ID) = 0;

    std::map<unsigned, IOParam> m_parameters;

    static std::map<std::type_index, int> s_peripheralCount;
    static void incrementPeriphCounter(const IOBase* p) {
        s_peripheralCount[typeid(p)]++;
        return;
    }
    static void decrementPeriphCounter(const IOBase* p) {
        Q_ASSERT(s_peripheralCount.at(typeid(p)) > 0);
        s_peripheralCount[typeid(p)]--;
    }

private:
    /**
     * @brief m_didUnregister
     * Debug variable to ensure that inheriting class called unregister() on delete.
     */
    bool m_didUnregister = false;
};
}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::IOParam);
