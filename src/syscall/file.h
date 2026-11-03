#pragma once

#include <type_traits>

#include "processorhandler.h"
#include "ripes_syscall.h"
#include "systemio.h"

namespace Ripes {

template <typename BaseSyscall>
class OpenSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  OpenSyscall()
      : BaseSyscall(
            "Open", "Opens a file from a path",
            {{0, "Pointer to null terminated string for the path"},
             {1, "flags" /* TODO(raccog): Add descriptions for each flag */}},
            {{0, "the file decriptor or -1 if an error occurred"}}) {}
  void execute() override {
    const AInt arg0 = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    const AInt arg1 = BaseSyscall::getArg(BaseSyscall::REG_FILE, 1);
    QByteArray string;
    char byte;
    AInt address = arg0;
    do {
      byte = static_cast<char>(
          ProcessorHandler::getMemory().readMemConst(address++, 1) & 0xFF);
      string.append(byte);
    } while (byte != '\0');
    if (string.endsWith('\0'))
      string.removeLast(); // Remove null-byte

    int ret = SystemIO::openFile(QString::fromUtf8(string), arg1);

    BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, ret);
  }
};

template <typename BaseSyscall>
class CloseSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  CloseSyscall()
      : BaseSyscall("Close", "Close a file",
                    {{0, "the file descriptor to close"}},
                    {{0, "0 on success or -1 if an error occurred"}}) {}
  void execute() override {
    const VInt arg0 = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, SystemIO::closeFile(arg0));
  }
};

template <typename BaseSyscall>
class LSeekSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  LSeekSyscall()
      : BaseSyscall("LSeek", "Seek to a position in a file",
                    {{0, "the file descriptor"},
                     {1, "the offset for the base"},
                     {2, "the base is the begining of the file (0), the "
                         "current position (1), or the end of the file (2)"}},
                    {{0, "the selected position from the beginning of the file "
                         "or -1 if an error occurred"}}) {}
  void execute() override {
    int result = SystemIO::seek(BaseSyscall::getArg(BaseSyscall::REG_FILE, 0),
                                BaseSyscall::getArg(BaseSyscall::REG_FILE, 1),
                                BaseSyscall::getArg(BaseSyscall::REG_FILE, 2));
    BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, result);
  }
};

template <typename BaseSyscall>
class ReadSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  ReadSyscall()
      : BaseSyscall("Read", "Read from a file descriptor into a buffer",
                    {{0, "the file descriptor"},
                     {1, "address of the buffer"},
                     {2, "maximum number of bytes to read"}},
                    {{0, "number of read bytes, 0 at end of file, or -1 if an "
                         "error occurred"}}) {}
  void execute() override {
    const int fd = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    AInt byteAddress = BaseSyscall::getArg(
        BaseSyscall::REG_FILE, 1); // destination of characters read from file
    const int length = BaseSyscall::getArg(BaseSyscall::REG_FILE, 2);
    QByteArray buffer;

    int retLength = SystemIO::readFromFile(fd, buffer, length);
    BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, retLength);

    if (retLength != -1) {
      // copy bytes from returned buffer into memory
      const char *dataptr = buffer.constData();
      while (retLength-- > 0) {
        ProcessorHandler::writeMem(byteAddress++, *dataptr++, sizeof(char));
      }
    }
  }
};

template <typename BaseSyscall>
class WriteSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  WriteSyscall()
      : BaseSyscall("Write", "Write to a file descriptor from a buffer",
                    {{0, "the file descriptor"},
                     {1, "address of the buffer"},
                     {2, "number of bytes to write"}},
                    {{0, "the number of bytes written"}}) {}
  void execute() override {
    const AInt byteAddress = BaseSyscall::getArg(
        BaseSyscall::REG_FILE, 1); // source of characters to write to file
    const int reqLength =
        BaseSyscall::getArg(BaseSyscall::REG_FILE, 2); // user-requested length
    if (reqLength < 0) {
      BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, -1);
      return;
    }
    QByteArray myBuffer;
    myBuffer.reserve(reqLength);
    for (int index = 0; index < reqLength; ++index) {
      myBuffer.append(static_cast<char>(
          ProcessorHandler::getMemory().readMemConst(byteAddress + index, 1) &
          0xFF));
    }

    const int retValue = SystemIO::writeToFile(
        BaseSyscall::getArg(BaseSyscall::REG_FILE, 0), myBuffer);
    BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, retValue);
  }
};

template <typename BaseSyscall>
class GetCWDSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  GetCWDSyscall()
      : BaseSyscall(
            "GetCWD",
            "Writes the path of the current working directory into a buffer",
            {{0, "address of the buffer to write into"},
             {1, "the length of the buffer"}},
            {{0, "-1 if the path is longer than the buffer"}}) {}
  void execute() override {
    const AInt byteAddress = BaseSyscall::getArg(
        BaseSyscall::REG_FILE, 0); // destination of characters read from file
    int index = 0;
    const int bufferSize = BaseSyscall::getArg(BaseSyscall::REG_FILE, 1);

    const QByteArray pwd = QDir::currentPath().toUtf8();

    if (pwd.length() >= bufferSize) {
      BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, -1);
      return;
    }
    BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, pwd.length());

    // copy bytes from returned buffer into memory
    while (index < pwd.length()) {
      ProcessorHandler::writeMem(byteAddress + index,
                                 static_cast<uint8_t>(pwd.at(index)),
                                 sizeof(char));
      index++;
    }
    ProcessorHandler::writeMem(byteAddress + index, 0, sizeof(char));
  }
};

template <typename BaseSyscall>
class FStatSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  FStatSyscall()
      : BaseSyscall(
            "FStat",
            "fstat is a system call that is used to determine information "
            "about a file based on its file descriptor.",
            {{0, " the file descriptor "}, {1, " pointer to a struct stat "}},
            {{0, "returns -1 if an error occurred"}}) {}
  void execute() override {
    const int fd = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    const AInt statAddress = BaseSyscall::getArg(BaseSyscall::REG_FILE, 1);

    SystemIO::FileStatus status;
    if (!SystemIO::fileStatus(fd, status)) {
      BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, -1);
      return;
    }

    // The buffer is a RISC-V Linux 'struct stat' (asm-generic layout, as used
    // by riscv-pk and libgloss' 'struct kernel_stat'). Only the fields up to
    // and including st_blocks are written; their layout is identical for RV32
    // and RV64. Fields which Ripes does not track are reported as 0.
    constexpr unsigned stModeOffset = 16;
    constexpr unsigned stNlinkOffset = 20;
    constexpr unsigned stSizeOffset = 48;
    constexpr unsigned statSize = 72;
    constexpr unsigned modeCharDevice = 0020000 | 0666; // S_IFCHR | rw-rw-rw-
    constexpr unsigned modeRegular = 0100000 | 0644;    // S_IFREG | rw-r--r--

    for (unsigned offset = 0; offset < statSize; offset += 4) {
      ProcessorHandler::writeMem(statAddress + offset, 0, 4);
    }
    const auto size = static_cast<uint64_t>(status.size);
    ProcessorHandler::writeMem(
        statAddress + stModeOffset,
        status.isCharDevice ? modeCharDevice : modeRegular, 4);
    ProcessorHandler::writeMem(statAddress + stNlinkOffset, 1, 4);
    ProcessorHandler::writeMem(statAddress + stSizeOffset, size & 0xFFFFFFFF,
                               4);
    ProcessorHandler::writeMem(statAddress + stSizeOffset + 4, size >> 32, 4);
    BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, 0);
  }
};
} // namespace Ripes
