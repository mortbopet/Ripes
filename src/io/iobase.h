#pragma once

#include <QVariant>
#include <QWidget>
#include <set>

#include "../assembler/program.h"
#include "binutils.h"
#include "serializers.h"

#include "VSRTL/external/cereal/include/cereal/cereal.hpp"

namespace Ripes {

/**
 * @brief cName
 * @returns a string escaped to be suitable as a source constant definition
 */
QString cName(const QString& name);

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

    template <class Archive>
    void serialize(Archive& archive) {
        archive(value);
    }
};

struct IOSymbol {
    Symbol name;
    VInt value;
};

struct RegDesc {
    enum class RW { R, W, RW };
    QString name;
    RW rw;
    unsigned bitWidth;
    AInt address;
    /**
     * @brief exported
     * if true, a constant symbol is generated that may be referenced in the assembler, which targets this register.
     */
    bool exported = false;
};

class IOBase : public QWidget {
    Q_OBJECT

public:
    IOBase(unsigned IOType /*ioregistry.h::IOType*/, QWidget* parent);
    virtual ~IOBase() {
        assert(m_didUnregister && "IO peripherals must call unregister() in their destructor!");
        unclaimPeripheralId(m_type, m_id);
    };

    const std::map<unsigned, IOParam>& parameters() const { return m_parameters; }
    virtual QString description() const = 0;

    /**
     * @brief name
     * @returns unique name for this specific component
     */
    virtual QString name() const;

    /**
     * @brief baseName
     * @return generic name for this IO type
     */
    virtual QString baseName() const = 0;

    /**
     * @brief registers
     * @return a description of the programmable interface of this peripheral
     */
    virtual const std::vector<RegDesc>& registers() const = 0;

    /**
     * @brief extraSymbols
     * @returns the set of extra symbols defined by this peripheral. Useful if some symbols aren't directly translated
     * from the register descriptions of this peripheral.
     */
    virtual const std::vector<IOSymbol>* extraSymbols() const { return nullptr; };

    /**
     * @brief setParameter
     * Attempt to set the parameter @p ID to @p value. Returns true if the value was set successfully.
     */
    virtual bool setParameter(unsigned ID, const QVariant& value);

    /**
     * @brief byteSize
     * Size of this peripheral, in bytes
     */
    virtual unsigned byteSize() const = 0;

    /**
     * Read/write functions from processor
     */
    virtual VInt ioRead(AInt offset, unsigned bytes) = 0;
    virtual void ioWrite(AInt offset, VInt value, unsigned bytes) = 0;

    /**
     * Read/write functions from peripheral to bus (memory/other periphs)
     */
    std::function<void(AInt, AInt, VInt)> memWrite;
    std::function<VInt(AInt, AInt)> memRead;

    unsigned iotype() const { return m_type; }
    unsigned id() const { return m_id; }
    void setID(unsigned id) {
        unclaimPeripheralId(m_type, m_id);
        m_id = claimPeripheralId(m_type, id);
    }

    /**
     * @brief serializedUniqueID
     * @returns a unique string identifying the type and ID of this peripheral, used during serialization of peripheral
     * state.
     */
    std::string serializedUniqueID() const { return std::to_string(iotype()) + "_" + std::to_string(id()); }

    template <class Archive>
    void serialize(Archive& archive) {
        int parameters = m_parameters.size();
        archive(cereal::make_nvp("nparameters", parameters));

        // Serialize a copy of m_parameters to avoid the real instance being overridden (we want to keep parameter name,
        // bounds, etc.).
        auto paramCopy = m_parameters;
        archive(cereal::make_nvp("parameters", paramCopy));
        for (const auto& p : paramCopy) {
            setParameter(p.first, p.second.value);
        }
    }

signals:
    /**
     * @brief regMapChanged
     * Should be emitted every time the register map of this peripheral changes.
     */
    void regMapChanged();

    /**
     * @brief sizeChanged
     * Should be emitted every time the memory size of this peripheral changes.
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

    /**
     * @brief paramsChanged
     * Emitted after a parameter has been changed _and_ recognized by the IOBase component.
     */
    void paramsChanged();

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
    unsigned m_id = UINT_MAX;

    static std::map<unsigned, std::set<unsigned>> s_peripheralIDs;
    static unsigned claimPeripheralId(const unsigned& ioType, int forcedID = -1) {
        unsigned id = 0;
        auto& currentIDs = s_peripheralIDs[ioType];
        if (forcedID != -1) {
            id = forcedID;
        } else {
            // Just scan linearly from 0 and up until a free ID is found. Not optimal, but # of peripherals is small so
            // no reason to overengineer.
            while (currentIDs.count(id) != 0) {
                id++;
            }
        }

        if (currentIDs.count(forcedID) != 0) {
            throw std::runtime_error("Trying to claim a peripheral ID already claimed");
        }

        currentIDs.insert(id);

        return id;
    }

    static void unclaimPeripheralId(const unsigned& ioType, unsigned id) {
        auto& currentIDs = s_peripheralIDs[ioType];
        auto it = currentIDs.find(id);
        Q_ASSERT(it != currentIDs.end());

        if (it == currentIDs.end()) {
            throw std::runtime_error("Trying to remove a non-claimed peripheral ID");
        }

        currentIDs.erase(it);
    }

private:
    /**
     * @brief m_didUnregister
     * Debug variable to ensure that inheriting class called unregister() on delete.
     */
    bool m_didUnregister = false;
    unsigned m_type;
};
}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::IOParam);
