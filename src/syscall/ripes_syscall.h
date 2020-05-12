#pragma once

#include <QMessageBox>
#include <QString>
#include <functional>
#include <map>
#include <memory>

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

    virtual uint32_t getArg(unsigned i) const = 0;
    /**
     * @brief setRet
     * ABI/ISA specific specialization of setting an argument register return value.
     * Argument index @p i is 0-indexed.
     */
    virtual void setRet(unsigned i, uint32_t value) const = 0;

protected:
    const QString m_name;
    const QString m_description;
    const std::map<unsigned, QString> m_argumentDescriptions;
    const std::map<unsigned, QString> m_returnDescriptions;
};

class SyscallManager {
public:
    /**
     * @brief execute
     * Executes the syscall identified by id. Throws out of range exception if syscall @p id is unknown.
     */
    void execute(int id) {
        if (m_syscalls.count(id) == 0) {
            QMessageBox::warning(nullptr, "Error",
                                 "Unknown system call: " + QString::number(id) +
                                     "\nRefer to \"Help->System calls\" for a list of support system calls.");
        } else {
            m_syscalls.at(id)->execute();
        }
    }

    const std::map<int, std::unique_ptr<Syscall>>& getSyscalls() const { return m_syscalls; }

protected:
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
