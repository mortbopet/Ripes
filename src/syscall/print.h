#pragma once

#include <type_traits>

#include "processorhandler.h"
#include "ripes_syscall.h"
#include "systemio.h"

namespace Ripes {

template <typename BaseSyscall>
class PrintIntSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  PrintIntSyscall()
      : BaseSyscall("PrintInt", "Prints an integer",
                    {{0, "integer to print"}}) {}
  void execute() {
    const VIntS arg0 = vsrtl::signextend<VInt, VIntS>(
        BaseSyscall::getArg(BaseSyscall::REG_FILE, 0),
        ProcessorHandler::currentISA()->bits());
    SystemIO::printString(QString::number(arg0));
  }
};

template <typename BaseSyscall>
class PrintFloatSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  PrintFloatSyscall()
      : BaseSyscall("PrintFloat", "Prints a floating point number",
                    {{0, "float to print"}}) {}
  void execute() {
    const VInt arg0 = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    auto *v_f = reinterpret_cast<const float *>(&arg0);
    SystemIO::printString(QString::number(static_cast<double>(*v_f)));
  }
};

template <typename BaseSyscall>
class PrintStrSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  PrintStrSyscall()
      : BaseSyscall("PrintString", "Prints a null-terminated string",
                    {{0, "address of the string"}}) {}
  void execute() {
    const VInt arg0 = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    QByteArray string;
    char byte;
    AInt address = arg0;
    do {
      byte = static_cast<char>(
          ProcessorHandler::getMemory().readMemConst(address++, 1) & 0xFF);
      string.append(byte);
    } while (byte != '\0');
    SystemIO::printString(QString::fromUtf8(string));
  }
};

template <typename BaseSyscall>
class PrintCharSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  PrintCharSyscall()
      : BaseSyscall(
            "PrintChar", "Prints an ascii character",
            {{0, "character to print (only lowest byte is considered)"}}) {}
  void execute() {
    const VInt arg0 = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    SystemIO::printString(QChar(static_cast<char>(arg0)));
  }
};

template <typename BaseSyscall>
class PrintHexSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  PrintHexSyscall()
      : BaseSyscall(
            "PrintIntHex",
            "Prints an integer (in hexdecimal format left-padded with zeroes)",
            {{0, "integer to print"}}) {}
  void execute() {
    const VInt arg0 = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    SystemIO::printString("0x" +
                          QString::number(arg0, 16).rightJustified(
                              ProcessorHandler::currentISA()->bytes(), '0'));
  }
};

template <typename BaseSyscall>
class PrintBinarySyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  PrintBinarySyscall()
      : BaseSyscall(
            "PrintIntBinary",
            "Prints an integer (in binary format left-padded with zeroes)",
            {{0, "integer to print"}}) {}
  void execute() {
    const VInt arg0 = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    SystemIO::printString("0b" +
                          QString::number(arg0, 2).rightJustified(
                              ProcessorHandler::currentISA()->bits(), '0'));
  }
};

template <typename BaseSyscall>
class PrintUnsignedSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  PrintUnsignedSyscall()
      : BaseSyscall("PrintIntUnsigned", "Prints an integer (unsigned)",
                    {{0, "integer to print"}}) {}
  void execute() {
    const VInt arg0 = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    SystemIO::printString(QString::number(arg0));
  }
};

} // namespace Ripes
