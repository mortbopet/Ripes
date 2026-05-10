#pragma once

#include <bitset>
#include <iostream>

#include "pseudoinstruction.h"
#include "rvisainfo_common.h"

// clang-format off
/**
 * Status register extension (Zicsr)
 * Instruction  parameters          Description
 * csrrw        rd, csr, rs1        rd = csr; csr = rs1
 * csrrs        rd, csr, rs1        rd = csr; csr = csr | rs1
 * csrrc        rd, csr, rs1        rd = csr; csr = csr & ~rs1
 * csrrwi       rd, csr, zimm       rd = csr; csr = zimm
 * csrrsi       rd, csr, zimm       rd = csr; csr = csr | zimm
 * csrrci       rd, csr, zimm       rd = csr; csr = csr & ~zimm
 *
 * Pseudo Instructions
 * csrr         rd, csr             rd = csr            >> expands to: csrrs rd, csr, x0
 * csrs         csr, rs1            csr = csr | rs1     >> expands to: csrrs x0, csr, rs1
 * csrc         csr, rs1            csr = csr & ~rs1    >> expands to: csrrc x0, csr, rs1
 * csrw         csr, rs1            csr = rs1           >> expands to: csrrw x0, csr, rs1
 * csrsi        csr, zimm           csr = csr | zimm    >> expands to: csrrsi x0, csr, zimm
 * csrci        csr, zimm           csr = csr & ~zimm   >> expands to: csrrci x0, csr, zimm
 * csrwi        csr, zimm           csr = zimm          >> expands to: csrrwi x0, csr, zimm
 */

namespace Ripes {
namespace RVISA {

namespace ExtZicsr {
  namespace TypeCSR {
    enum class Funct3 : unsigned {
      ECALL  = 0b000,
      CSRRW  = 0b001,
      CSRRS  = 0b010,
      CSRRC  = 0b011,
      CSRRWI = 0b101,
      CSRRSI = 0b110,
      CSRRCI = 0b111
    };

    /// The RISC-V csr field contains a status register index.
    /// It is defined as a 12-bit field in bits 20-31 of the instruction
    template <unsigned tokenIndex>
    struct RegCSR : public CSR_Reg<RegCSR<tokenIndex>, tokenIndex, BitRange<20, 31>> {
      constexpr static std::string_view getName() { return "csr"; }
    };

    template <unsigned tokenIndex>
    struct Imm5 : public Imm<tokenIndex, 5, Repr::Unsigned, ImmPart<0, 15, 19>> {};

    /// An I-Type RISC-V instruction
    template <typename InstrImpl, Funct3 funct3, template <unsigned> class Field2>
    struct Instr : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<OpcodeID::SYSTEM>,
                              OpPartFunct3<static_cast<unsigned>(funct3)>> {};
      struct Fields : public FieldSet<RegRd, RegCSR, Field2> {};
    };

    template <typename InstrImpl, Funct3 funct3>
    using InstrReg = Instr<InstrImpl, funct3, RegRs1>;
    template <typename InstrImpl, Funct3 funct3>
    using InstrImm = Instr<InstrImpl, funct3, Imm5>;
    
    struct Csrrs : public InstrReg<Csrrs, Funct3::CSRRS> {
      constexpr static std::string_view NAME = "csrrs";
    };
    struct Csrrw : public InstrReg<Csrrw, Funct3::CSRRW> {
      constexpr static std::string_view NAME = "csrrw";
    };
    struct Csrrc : public InstrReg<Csrrc, Funct3::CSRRC> {
      constexpr static std::string_view NAME = "csrrc";
    };
    
    struct Csrrsi : public InstrImm<Csrrsi, Funct3::CSRRSI> {
      constexpr static std::string_view NAME = "csrrsi";
    };
    struct Csrrwi : public InstrImm<Csrrwi, Funct3::CSRRWI> {
      constexpr static std::string_view NAME = "csrrwi";
    };
    struct Csrrci : public InstrImm<Csrrci, Funct3::CSRRCI> {
      constexpr static std::string_view NAME = "csrrci";
    };
  }

  namespace TypePseudo {
    template <unsigned tokenIndex>
    struct PseudoReg : public Ripes::PseudoReg<tokenIndex, RV_GPRInfo> {};

    template <typename PseudoInstrImpl, bool x0_is_rd, template <unsigned> class Field2>
    struct PseudoAlias : public PseudoInstruction<PseudoInstrImpl> {
      struct Fields : public FieldSet<PseudoReg, Field2> {};

      static Result<std::vector<LineTokens>>
      expander(const PseudoInstruction<PseudoInstrImpl> &,
               const TokenizedSrcLine &line, const SymbolMap &) {
        LineTokensVec v;
        // {pseudo} rd(/csr), rs1/imm(/csr)  >> expands to: {real} rd, csr, rs1/imm
        v.push_back( LineTokens()
            << QString(PseudoInstrImpl::PSEUDO_FUNC.data()) );
        
        if constexpr (x0_is_rd) {
          v.back() << Token("x0");
        }

        v.back() << line.tokens.at(1)
                 << line.tokens.at(2);
        
        if constexpr (!x0_is_rd) {
          v.back() << Token("x0");
        }
        
        return v;

      }
    };
    
    template <typename PseudoInstrImpl, bool x0_is_rd>
    using PseudoAliasReg = PseudoAlias<PseudoInstrImpl, x0_is_rd, PseudoReg>;
    template <typename PseudoInstrImpl, bool x0_is_rd>
    using PseudoAliasImm = PseudoAlias<PseudoInstrImpl, x0_is_rd, PseudoImm>;

    struct Csrr : public PseudoAliasReg<Csrr, false> {
      constexpr static std::string_view NAME = "csrr";
      constexpr static std::string_view PSEUDO_FUNC = "csrrs";
    };
    struct Csrs : public PseudoAliasReg<Csrs, true> {
      constexpr static std::string_view NAME = "csrs";
      constexpr static std::string_view PSEUDO_FUNC = "csrrs";
    };
    struct Csrc : public PseudoAliasReg<Csrc, true> {
      constexpr static std::string_view NAME = "csrc";
      constexpr static std::string_view PSEUDO_FUNC = "csrrc";
    };
    struct Csrw : public PseudoAliasReg<Csrw, true> {
      constexpr static std::string_view NAME = "csrw";
      constexpr static std::string_view PSEUDO_FUNC = "csrrw";
    };

    struct Csrsi : public PseudoAliasImm<Csrsi, true> {
      constexpr static std::string_view NAME = "csrsi";
      constexpr static std::string_view PSEUDO_FUNC = "csrrsi";
    };
    struct Csrci : public PseudoAliasImm<Csrci, true> {
      constexpr static std::string_view NAME = "csrci";
      constexpr static std::string_view PSEUDO_FUNC = "csrrci";
    };
    struct Csrwi : public PseudoAliasImm<Csrwi, true> {
      constexpr static std::string_view NAME = "csrwi";
      constexpr static std::string_view PSEUDO_FUNC = "csrrwi";
    };

// clang-format on
} // namespace TypePseudo
}; // namespace ExtZicsr

} // namespace RVISA
} // namespace Ripes
