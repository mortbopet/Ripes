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
};

struct Mulh : public Instr32<Mulh, Funct3::MULH> {
  constexpr static std::string_view NAME = "mulh";
};

struct Mulhsu : public Instr32<Mulhsu, Funct3::MULHSU> {
  constexpr static std::string_view NAME = "mulhsu";
};

struct Mulhu : public Instr32<Mulhu, Funct3::MULHU> {
  constexpr static std::string_view NAME = "mulhu";
};

struct Div : public Instr32<Div, Funct3::DIV> {
  constexpr static std::string_view NAME = "div";
};

struct Divu : public Instr32<Divu, Funct3::DIVU> {
  constexpr static std::string_view NAME = "divu";
};

struct Rem : public Instr32<Rem, Funct3::REM> {
  constexpr static std::string_view NAME = "rem";
};

struct Remu : public Instr32<Remu, Funct3::REMU> {
  constexpr static std::string_view NAME = "remu";
};

struct Mulw : public Instr64<Mulw, Funct3::MUL> {
  constexpr static std::string_view NAME = "mulw";
};

struct Divw : public Instr64<Divw, Funct3::DIV> {
  constexpr static std::string_view NAME = "divw";
};

struct Divuw : public Instr64<Divuw, Funct3::DIVU> {
  constexpr static std::string_view NAME = "divuw";
};

struct Remw : public Instr64<Remw, Funct3::REM> {
  constexpr static std::string_view NAME = "remw";
};

struct Remuw : public Instr64<Remuw, Funct3::REMU> {
  constexpr static std::string_view NAME = "remuw";
};

} // namespace TypeR

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions);

} // namespace ExtM

} // namespace RVISA
} // namespace Ripes
