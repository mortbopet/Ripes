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
        SyscallStatusManager::setStatusTimed("Handling system call: " + syscall->name() + " (" + QString::number(id) +
                                             ")");
        syscall->execute();
        SyscallStatusManager::clearStatus();
        return true;
    }
}

}  // namespace Ripes
