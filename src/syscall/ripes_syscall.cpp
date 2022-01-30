#include "ripes_syscall.h"

#include "processorhandler.h"

namespace Ripes {

bool SyscallManager::execute(SyscallID id) {
    if (m_syscalls.count(id) == 0) {
        postToGUIThread([=] {
            QMessageBox::warning(
                nullptr, "Error",
                "Unknown system call in register '" +
                    ProcessorHandler::currentISA()->regAlias(ProcessorHandler::currentISA()->syscallReg()) + "': " +
                    QString::number(id) + "\nRefer to \"Help->System calls\" for a list of support system calls.");
        });
        return false;
    } else {
        const auto& syscall = m_syscalls.at(id);
        const QString& syscallName = syscall->name();
        postToGUIThread([=] {
            // We don't have a good way of making non-permanent status timers pseudo-permanent until explicitly
            // cleared... The best way to do so is to just have a very large timeout.
            SyscallStatusManager::setStatusTimed(
                "Handling system call: " + syscallName + " (" + QString::number(id) + ")", 99999999);
        });
        syscall->execute();
        postToGUIThread([=] { SyscallStatusManager::clearStatus(); });
        return true;
    }
}

}  // namespace Ripes
