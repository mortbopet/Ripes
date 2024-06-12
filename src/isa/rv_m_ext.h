#pragma once

#include "pseudoinstruction.h"
#include "rv_i_ext.h"
#include "rvisainfo_common.h"

namespace Ripes {
namespace RVISA {

namespace ExtM {

namespace TypeR {

using ExtI::TypeR::Funct7;

enum class Funct3 {
  MUL = 0b000,
  MULH = 0b001,
  MULHSU = 0b010,
  MULHU = 0b011,
  DIV = 0b100,
  DIVU = 0b101,
  REM = 0b110,
  REMU = 0b111
};

template <typename InstrImpl, Funct3 funct3>
using Instr32 =
    ExtI::TypeR::Instr<InstrImpl, OpcodeID::OP, Funct3, funct3, Funct7::M_EXT>;
template <typename InstrImpl, Funct3 funct3>
using Instr64 = ExtI::TypeR::Instr<InstrImpl, OpcodeID::OP32, Funct3, funct3,
                                   Funct7::M_EXT>;

struct Mul : public Instr32<Mul, Funct3::MUL> {
  constexpr static std::string_view NAME = "mul";
  constexpr static std::string_view DESC = "Multiply Signed x Signed";
};

struct Mulh : public Instr32<Mulh, Funct3::MULH> {
  constexpr static std::string_view NAME = "mulh";
  constexpr static std::string_view DESC = "Multiply High Signed x Signed";
};

struct Mulhsu : public Instr32<Mulhsu, Funct3::MULHSU> {
  constexpr static std::string_view NAME = "mulhsu";
  constexpr static std::string_view DESC = "Multiply High Signed x Unsigned";
};

struct Mulhu : public Instr32<Mulhu, Funct3::MULHU> {
  constexpr static std::string_view NAME = "mulhu";
  constexpr static std::string_view DESC = "Multiply High Unsigned x Unsigned";
};

struct Div : public Instr32<Div, Funct3::DIV> {
  constexpr static std::string_view NAME = "div";
  constexpr static std::string_view DESC = "Divide";
};

struct Divu : public Instr32<Divu, Funct3::DIVU> {
  constexpr static std::string_view NAME = "divu";
  constexpr static std::string_view DESC = "Divide Unsigned";
};

struct Rem : public Instr32<Rem, Funct3::REM> {
  constexpr static std::string_view NAME = "rem";
  constexpr static std::string_view DESC = "Remainder";
};

struct Remu : public Instr32<Remu, Funct3::REMU> {
  constexpr static std::string_view NAME = "remu";
  constexpr static std::string_view DESC = "Remainder Unsigned";
};

struct Mulw : public Instr64<Mulw, Funct3::MUL> {
  constexpr static std::string_view NAME = "mulw";
  constexpr static std::string_view DESC = "Multiply Signed x Signed Wide";
};

struct Divw : public Instr64<Divw, Funct3::DIV> {
  constexpr static std::string_view NAME = "divw";
  constexpr static std::string_view DESC = "Divide Wide";
};

struct Divuw : public Instr64<Divuw, Funct3::DIVU> {
  constexpr static std::string_view NAME = "divuw";
  constexpr static std::string_view DESC = "Divide Unsigned Wide";
};

struct Remw : public Instr64<Remw, Funct3::REM> {
  constexpr static std::string_view NAME = "remw";
  constexpr static std::string_view DESC = "Remainder Wide";
};

struct Remuw : public Instr64<Remuw, Funct3::REMU> {
  constexpr static std::string_view NAME = "remuw";
  constexpr static std::string_view DESC = "Remainder Unsigned Wide";
};

} // namespace TypeR

} // namespace ExtM

} // namespace RVISA
} // namespace Ripes
