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
            const std::map<unsigned, QString>& argumentDescriptions = std::map<unsigned, QString>())
        : m_name(name), m_description(description), m_argumentDescriptions(argumentDescriptions) {}
    virtual ~Syscall() {}
    const QString& name() const { return m_name; }
    const QString& description() const { return m_description; }

    virtual void execute() = 0;

    virtual uint32_t getArg(unsigned i) const = 0;

protected:
    const QString m_name;
    const QString m_description;
    const std::map<unsigned, QString> m_argumentDescriptions;
};

class SyscallManager {
public:
    /**
     * @brief execute
     * Executes the syscall identified by id. Throws out of range exception if syscall @p id is unknown.
     */
    void execute(int id) {
        if (m_syscalls.count(id) == 0) {
            QMessageBox::warning(nullptr, "Error", "Unknown system call: " + QString::number(id));
        } else {
            m_syscalls.at(id)->execute();
        }
    }

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
