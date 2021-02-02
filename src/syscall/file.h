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
        : BaseSyscall("Open", "Opens a file from a path",
                      {{0, "Pointer to null terminated string for the path"}, {1, "flags"}},
                      {{0, "the file decriptor or -1 if an error occurred"}}) {}
    void execute() {
        const uint32_t arg0 = BaseSyscall::getArg(RegisterFileType::GPR,0);
        const uint32_t arg1 = BaseSyscall::getArg(RegisterFileType::GPR,1);
        QByteArray string;
        char byte;
        unsigned int address = arg0;
        do {
            byte = static_cast<char>(ProcessorHandler::get()->getMemory().readMemConst(address++, 1) & 0xFF);
            string.append(byte);
        } while (byte != '\0');

        int ret = SystemIO::openFile(QString::fromUtf8(string), arg1);

        BaseSyscall::setRet(RegisterFileType::GPR,0, ret);
    }
};

template <typename BaseSyscall>
class CloseSyscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    CloseSyscall() : BaseSyscall("Close", "Close a file", {{0, "the file descriptor to close"}}) {}
    void execute() {
        const uint32_t arg0 = BaseSyscall::getArg(RegisterFileType::GPR,0);
        SystemIO::closeFile(arg0);
    }
};

template <typename BaseSyscall>
class LSeekSyscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    LSeekSyscall()
        : BaseSyscall(
              "LSeek", "Seek to a position in a file",
              {{0, "the file descriptor"},
               {1, "the offset for the base"},
               {2, "the base is the begining of the file (0), the current position (1), or the end of the file (2)"}},
              {{0, "the selected position from the beginning of the file or -1 if an error occurred"}}) {}
    void execute() {
        int result = SystemIO::seek(BaseSyscall::getArg(RegisterFileType::GPR,0), BaseSyscall::getArg(RegisterFileType::GPR,1), BaseSyscall::getArg(RegisterFileType::GPR,2));
        BaseSyscall::setRet(RegisterFileType::GPR,0, result);
    }
};

template <typename BaseSyscall>
class ReadSyscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    ReadSyscall()
        : BaseSyscall(
              "Read", "Read from a file descriptor into a buffer",
              {{0, "the file descriptor"}, {1, "address of the buffer"}, {2, "maximum number of bytes to read"}},
              {{0, "number of read bytes or -1 if an error occurred"}}) {}
    void execute() {
        const int fd = BaseSyscall::getArg(RegisterFileType::GPR,0);
        int byteAddress = BaseSyscall::getArg(RegisterFileType::GPR,1);  // destination of characters read from file
        const int length = BaseSyscall::getArg(RegisterFileType::GPR,2);
        QByteArray buffer;

        int retLength = SystemIO::readFromFile(fd, buffer, length);
        BaseSyscall::setRet(RegisterFileType::GPR,0, retLength);

        if (retLength != -1) {
            // copy bytes from returned buffer into memory
            const char* dataptr = buffer.constData();  // QString::data contains a possible null termination '\0'
                                                       // character (present if reading from stdin and not from a file)
            while (retLength-- > 0) {
                ProcessorHandler::get()->writeMem(byteAddress++, *dataptr++, sizeof(char));
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
                      {{0, "the file descriptor"}, {1, "address of the buffer"}, {2, "number of bytes to write"}},
                      {{0, "the number of bytes written"}}) {}
    void execute() {
        const int byteAddress = BaseSyscall::getArg(RegisterFileType::GPR,1);  // source of characters to write to file
        const int reqLength = BaseSyscall::getArg(RegisterFileType::GPR,2);    // user-requested length
        if (reqLength < 0) {
            BaseSyscall::setRet(RegisterFileType::GPR,0, -1);
            return;
        }
        int index = 0;
        QString myBuffer;

        do {
            myBuffer.append(
                static_cast<char>(ProcessorHandler::get()->getMemory().readMemConst(byteAddress + index++, 1) & 0xFF));
        } while (index < reqLength);

        const int retValue = SystemIO::writeToFile(BaseSyscall::getArg(RegisterFileType::GPR,0), myBuffer, reqLength);
        BaseSyscall::setRet(RegisterFileType::GPR,0, retValue);
    }
};

template <typename BaseSyscall>
class GetCWDSyscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    GetCWDSyscall()
        : BaseSyscall("GetCWD", "Writes the path of the current working directory into a buffer",
                      {{0, "the buffer to write into"}, {1, "the length of the buffer"}},
                      {{0, "-1 if the path is longer than the buffer"}}) {}
    void execute() {
        const int byteAddress = BaseSyscall::getArg(RegisterFileType::GPR,0);  // destination of characters read from file
        int index = 0;
        const int bufferSize = BaseSyscall::getArg(RegisterFileType::GPR,1);

        const QString pwd = QDir::currentPath();

        if (pwd.length() > bufferSize) {
            BaseSyscall::setRet(RegisterFileType::GPR,0, -1);
            return;
        } else {
            BaseSyscall::setRet(RegisterFileType::GPR,0, pwd.length());
        }

        // copy bytes from returned buffer into memory
        while (index < pwd.length()) {
            ProcessorHandler::get()->writeMem(byteAddress, pwd.at(index++).toLatin1(), sizeof(char));
        }
    }
};

template <typename BaseSyscall>
class FStatSyscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    FStatSyscall()
        : BaseSyscall(
              "FStat",
              "fstat is a system call that is used to determine information about a file based on its file descriptor.",
              {{0, " the file descriptor "}, {1, " pointer to a struct stat "}},
              {{0, "returns -1 if an error occurred"}}) {}
    void execute() {
        /// @todo: Figure out whether we want to handle fstat. For now, most syscall's seems to execute fine with just
        /// null-values in the fstat return struct - this is ofcourse not a valid mode of execution, but for the current
        /// state of development, it'll have to do.
        BaseSyscall::setRet(RegisterFileType::GPR,0, 0);
    }
};
}  // namespace Ripes
