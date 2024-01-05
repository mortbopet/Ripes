#pragma once

#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QString>
#include <QThread>

#include <functional>
#include <map>
#include <memory>

#include "../isa/isainfo.h"
#include "isa/isa_types.h"
#include "statusmanager.h"

namespace Ripes {

/**
 * @brief The Syscall class
 * Base class for all system calls. Must be specialized by an ISA/ABI specific
 * system call class. This class shall define getArg() based on the given ABI.
 */
class Syscall {
public:
  using ArgIdx = unsigned;
  Syscall(const QString &name, const QString &description = QString(),
          const std::map<ArgIdx, QString> &argumentDescriptions =
              std::map<ArgIdx, QString>(),
          const std::map<ArgIdx, QString> &returnDescriptions =
              std::map<ArgIdx, QString>())
      : m_name(name), m_description(description),
        m_argumentDescriptions(argumentDescriptions),
        m_returnDescriptions(returnDescriptions) {}
  virtual ~Syscall() {}
  const QString &name() const { return m_name; }
  const QString &description() const { return m_description; }
  const std::map<ArgIdx, QString> &argumentDescriptions() const {
    return m_argumentDescriptions;
  }
  const std::map<ArgIdx, QString> &returnDescriptions() const {
    return m_returnDescriptions;
  }

  virtual void execute() = 0;

  /**
   * @brief getArg
   * ABI specific specialization of returning an argument register value.
   * Argument index @p i is in range [0; max arg. registers] for the given ABI.
   * @returns value of the specified argument register.
   */

  virtual VInt getArg(const std::string_view &rfid, ArgIdx i) const = 0;
  /**
   * @brief setRet
   * ABI specific specialization of setting an argument register return value.
   * Argument index @p i is in range [0; max arg. registers] for the given ABI.
   */
  virtual void setRet(const std::string_view &rfid, ArgIdx i,
                      VInt value) const = 0;

protected:
  const QString m_name;
  const QString m_description;
  const std::map<ArgIdx, QString> m_argumentDescriptions;
  const std::map<ArgIdx, QString> m_returnDescriptions;
};

/**
 * @brief The SyscallManager class
 *
 * It is expected that the syscallManager can be called outside of the main GUI
 * thread. As such, all syscalls who require GUI interaction must handle this
 * explicitly.
 */
class SyscallManager {
public:
  using SyscallID = int;
  /**
   * @brief execute
   * Executes the syscall identified by id. Throws out of range exception if
   * syscall @p id is unknown.
   */
  bool execute(SyscallID id);

  const std::map<SyscallID, std::unique_ptr<Syscall>> &getSyscalls() const {
    return m_syscalls;
  }

protected:
  SyscallManager() {}
  std::map<SyscallID, std::unique_ptr<Syscall>> m_syscalls;
};

template <class T>
class SyscallManagerT : public SyscallManager {
  static_assert(std::is_base_of<Syscall, T>::value);

public:
  template <class T_Syscall>
  void emplace(SyscallID id) {
    static_assert(std::is_base_of<T, T_Syscall>::value);
    assert(m_syscalls.count(id) == 0);
    m_syscalls.emplace(id, std::make_unique<T_Syscall>());
  }
};

} // namespace Ripes
