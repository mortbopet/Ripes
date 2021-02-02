#pragma once

#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QString>
#include <QThread>

#include <functional>
#include <map>
#include <memory>

#include "../isa/isainfo.h"
#include "statusmanager.h"

namespace Ripes {

/**
 * @brief The Syscall class
 * Base class for all system calls. Must be specialized by an ISA/ABI specific system call class. This class shall
 * define getArg() based on the given ABI.
 */
class Syscall {
public:
    Syscall(const QString& name, const QString& description = QString(),
            const std::map<unsigned, QString>& argumentDescriptions = std::map<unsigned, QString>(),
            const std::map<unsigned, QString>& returnDescriptions = std::map<unsigned, QString>())
        : m_name(name),
          m_description(description),
          m_argumentDescriptions(argumentDescriptions),
          m_returnDescriptions(returnDescriptions) {}
    virtual ~Syscall() {}
    const QString& name() const { return m_name; }
    const QString& description() const { return m_description; }
    const std::map<unsigned, QString>& argumentDescriptions() const { return m_argumentDescriptions; }
    const std::map<unsigned, QString>& returnDescriptions() const { return m_returnDescriptions; }

    virtual void execute() = 0;

    /**
     * @brief getArg
     * ABI/ISA specific specialization of returning an argument register value.
     * Argument index @p i is 0-indexed. @returns value of the specified argument register.
     */

    virtual uint32_t getArg(RegisterFileType rfid, unsigned i) const = 0;
    /**
     * @brief setRet
     * ABI/ISA specific specialization of setting an argument register return value.
     * Argument index @p i is 0-indexed.
     */
    virtual void setRet(RegisterFileType rfid, unsigned i, uint32_t value) const = 0;

protected:
    const QString m_name;
    const QString m_description;
    const std::map<unsigned, QString> m_argumentDescriptions;
    const std::map<unsigned, QString> m_returnDescriptions;
};

/**
 * @brief SyscallStatusManager
 * Any syscall status messages may be posted to the following class. This class is connected to a status bar widget
 * in the mainwindow and subsequently displays all messages to the user.
 */
StatusManager(Syscall);

/**
 * @brief The SyscallManager class
 *
 * It is expected that the syscallManager can be called outside of the main GUI thread. As such, all syscalls who
 * require GUI interaction must handle this explicitely.
 */
class SyscallManager {
public:
    /**
     * @brief execute
     * Executes the syscall identified by id. Throws out of range exception if syscall @p id is unknown.
     */
    bool execute(int id);

    const std::map<int, std::unique_ptr<Syscall>>& getSyscalls() const { return m_syscalls; }

protected:
    /**
     * @brief postToGUIThread
     * Schedules the execution of @param fun in the GUI thread.
     * @param connection type.
     */
    template <typename F>
    static void postToGUIThread(F&& fun, Qt::ConnectionType type = Qt::QueuedConnection) {
        auto* obj = QAbstractEventDispatcher::instance(qApp->thread());
        Q_ASSERT(obj);
        QMetaObject::invokeMethod(obj, std::forward<F>(fun), type);
    }

    SyscallManager() {}
    std::map<int, std::unique_ptr<Syscall>> m_syscalls;
};

template <class T>
class SyscallManagerT : public SyscallManager {
    static_assert(std::is_base_of<Syscall, T>::value);

public:
    template <class T_Syscall>
    void emplace(int id) {
        static_assert(std::is_base_of<T, T_Syscall>::value);
        assert(m_syscalls.count(id) == 0);
        m_syscalls.emplace(id, std::make_unique<T_Syscall>());
    }
};

}  // namespace Ripes
