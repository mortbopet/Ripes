#include "ripes_syscall.h"

#include "processorhandler.h"
#include "systemio.h"

namespace Ripes {

bool SyscallManager::execute(SyscallID id) {
  if (!m_syscalls.contains(id)) {
    postToGUIThread([id] {
      if (auto reg = ProcessorHandler::currentISA()->syscallReg();
          reg.has_value()) {
        const QString msg = "Unknown system call in register '" +
                            reg->file->regAlias(reg->index) +
                            "': " + QString::number(id) +
                            "\nRefer to \"Help->System calls\" for a list "
                            "of support system "
                            "calls.";
        // Don't block headless (CLI) runs on a modal dialog.
        if (QApplication::topLevelWidgets().isEmpty())
          SystemIO::printString("\nError: " + msg + "\n");
        else
          QMessageBox::warning(nullptr, "Error", msg);
      }
    });
    return false;
  }
  const auto &syscall = m_syscalls.at(id);
  const QString &syscallName = syscall->name();
  postToGUIThread([syscallName, id] {
    // We don't have a good way of making non-permanent status timers
    // pseudo-permanent until explicitly cleared... The best way to do so is
    // to just have a very large timeout.
    SyscallStatusManager::setStatusTimed(
        "Handling system call: " + syscallName + " (" + QString::number(id) +
            ")",
        99999999);
  });
  syscall->execute();
  postToGUIThread([] { SyscallStatusManager::clearStatus(); });
  return true;
}

} // namespace Ripes
