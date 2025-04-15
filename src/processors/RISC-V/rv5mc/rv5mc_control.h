#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"
#include "processors/RISC-V/riscv.h"
#include <magic_enum/magic_enum.hpp>


namespace vsrtl {
namespace core {
using namespace Ripes;

namespace rv5mc {
  enum class AluSrc1 { REG1, PC, PCOLD };
  enum class AluSrc2 { REG2, IMM, INPC };
  enum class ALUControl { ADD, SUB, INSTRUCTION_DEPENDENT };

  enum class FSMState {
    IF, ID, //States common to all operations
    EX, //Generic execution state
    //Specific States by Execution Type
    //Jump operations
    EXJAL, EXJALR,
    EXCJE, EXCJNE, EXCJGE, EXCJLT,
    EXCJLTU, EXCJGEU,

    //R-Type
    //Arimethic
    EXADD, EXSUB, EXSLT, EXSLTU,
    EXAND, EXOR, EXXOR, EXSLL,
    EXSRL, EXSRA, EXMUL, EXMULH,
    EXMULHSU, EXMULHU, EXDIV, EXDIVU,
    EXREM, EXREMU, EXMULW, EXDIVW,
    EXDIVUW, EXREMW, EXREMUW, EXADDW,
    EXSUBW, EXSLLW, EXSRLW, EXSRAW,

    //I-Type
    //Arimethic
    EXADDI, EXSLTI,
    EXSLTIU, EXANDI, EXORI, EXXORI,
    EXSLLI, EXSRLI, EXSRAI, EXLUI,
    EXAUIP, EXADDIW, EXSLLIW, EXSRLIW,
    EXSRAIW,

    //Ecall
    EXECALL,
    //Memory load
    EXMEMOP,

    MEM, //Generic memory state
    //Concrete states of memory

    MEMJALR,

    //R-Type
    //Arimethic
    MEMALINS,

    //I-Type
    //Memory Load
    MEMLB, MEMLH, MEMLW, MEMLBU,
    MEMLHU, MEMLWU, MEMLD,
    //Memory Store
    MEMSB, MEMSH, MEMSW, MEMSD,
    WB,//Generic Write Back state
    //Specific Write Back states
    //I-Type
    //Memory Load
    WBMEMLOAD,

    //Invalid
    INVALID
  };
  struct StateSignals {
    bool ir_write = false;
    bool pc_write = false;
    bool branch = false;
    PcSrc pc_src = PcSrc::ALU;
    bool reg_write = false;
    RegWrSrc reg_src = RegWrSrc::ALURES;
    AluSrc1 alu_op1_src = AluSrc1::PC;
    AluSrc2 alu_op2_src = AluSrc2::IMM;
    ALUControl alu_control = ALUControl::ADD;
    bool mem_read = false;
    bool mem_write = false;
    MemOp mem_op = MemOp::LB; // TODO: remove
    bool ecall = false;
  };
  typedef FSMState (*TransitionFunc)(RVInstr);

  struct StateInfo {
    StateSignals outs;
    TransitionFunc transitions;
  };
}

using namespace rv5mc;

class RVMCControl : public Component {
public:
  /* clang-format off */
  std::vector<StateInfo> states;

  static CompOp do_comp_ctrl(RVInstr opc) { // TODO: rename?
    switch(opc){
    case RVInstr::BEQ: return CompOp::EQ;
    case RVInstr::BNE: return CompOp::NE;
    case RVInstr::BLT: return CompOp::LT;
    case RVInstr::BGE: return CompOp::GE;
    case RVInstr::BLTU: return CompOp::LTU;
    case RVInstr::BGEU: return CompOp::GEU;
    default: return CompOp::NOP;
    }
  }

  static ALUOp do_alu_ctrl(RVInstr opc) { // TODO: rename?
    switch(opc) {
    case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU: case RVInstr::LHU:
    case RVInstr::SB: case RVInstr::SH: case RVInstr::SW: case RVInstr::LWU: case RVInstr::LD:
    case RVInstr::SD:
      return ALUOp::ADD; // not used (alu_control)
    case RVInstr::LUI:
      return ALUOp::LUI;
    case RVInstr::JAL: case RVInstr::JALR: case RVInstr::AUIPC:
    case RVInstr::ADD: case RVInstr::ADDI:
    case RVInstr::BEQ: case RVInstr::BNE: case RVInstr::BLT:
    case RVInstr::BGE: case RVInstr::BLTU: case RVInstr::BGEU:
      return ALUOp::ADD;
    case RVInstr::SUB: return ALUOp::SUB;
    case RVInstr::SLT: case RVInstr::SLTI:
      return ALUOp::LT;
    case RVInstr::SLTU: case RVInstr::SLTIU:
      return ALUOp::LTU;
    case RVInstr::XOR: case RVInstr::XORI:
      return ALUOp::XOR;
    case RVInstr::OR: case RVInstr::ORI:
      return ALUOp::OR;
    case RVInstr::AND: case RVInstr::ANDI:
      return ALUOp::AND;
    case RVInstr::SLL: case RVInstr::SLLI:
      return ALUOp::SL;
    case RVInstr::SRL: case RVInstr::SRLI:
      return ALUOp::SRL;
    case RVInstr::SRA: case RVInstr::SRAI:
      return ALUOp::SRA;
    case RVInstr::MUL   : return ALUOp::MUL;
    case RVInstr::MULH  : return ALUOp::MULH;
    case RVInstr::MULHU : return ALUOp::MULHU;
    case RVInstr::MULHSU: return ALUOp::MULHSU;
    case RVInstr::DIV   : return ALUOp::DIV;
    case RVInstr::DIVU  : return ALUOp::DIVU;
    case RVInstr::REM   : return ALUOp::REM;
    case RVInstr::REMU  : return ALUOp::REMU;
    case RVInstr::ADDIW : return ALUOp::ADDW;
    case RVInstr::SLLIW : return ALUOp::SLW;
    case RVInstr::SRLIW : return ALUOp::SRLW;
    case RVInstr::SRAIW : return ALUOp::SRAW;
    case RVInstr::ADDW  : return ALUOp::ADDW ;
    case RVInstr::SUBW  : return ALUOp::SUBW ;
    case RVInstr::SLLW  : return ALUOp::SLW ;
    case RVInstr::SRLW  : return ALUOp::SRLW ;
    case RVInstr::SRAW  : return ALUOp::SRAW ;
    case RVInstr::MULW  : return ALUOp::MULW ;
    case RVInstr::DIVW  : return ALUOp::DIVW ;
    case RVInstr::DIVUW : return ALUOp::DIVUW;
    case RVInstr::REMW  : return ALUOp::REMW ;
    case RVInstr::REMUW : return ALUOp::REMUW;

    default: return ALUOp::NOP;
    }
  }

  static FSMState do_EX_stateFromOPC(RVInstr opc) {
    switch(opc){
    case RVInstr::JAL:
      return FSMState::EXJAL;
    case RVInstr::JALR:
      return FSMState::EXJALR;
    case RVInstr::BEQ:
      return FSMState::EXCJE;
    case RVInstr::BNE:
      return FSMState::EXCJNE;
    case RVInstr::BLT:
      return FSMState::EXCJLT;
    case RVInstr::BGE:
      return FSMState::EXCJGE;
    case RVInstr::BLTU:
      return FSMState::EXCJLTU;
    case RVInstr::BGEU:
      return FSMState::EXCJGEU;
    case RVInstr::ADD:
      return FSMState::EXADD;
    case RVInstr::SUB:
      return FSMState::EXSUB;
    case RVInstr::SLT:
      return FSMState::EXSLT;
    case RVInstr::SLTU:
      return FSMState::EXSLTU;
    case RVInstr::AND:
      return FSMState::EXAND;
    case RVInstr::OR:
      return FSMState::EXOR;
    case RVInstr::XOR:
      return FSMState::EXXOR;
    case RVInstr::SLL:
      return FSMState::EXSLL;
    case RVInstr::SRL:
      return FSMState::EXSRL;
    case RVInstr::SRA:
      return FSMState::EXSRA;
    case RVInstr::ADDI:
      return FSMState::EXADDI;
    case RVInstr::SLTI:
      return FSMState::EXSLTI;
    case RVInstr::SLTIU:
      return FSMState::EXSLTIU;
    case RVInstr::ANDI:
      return FSMState::EXANDI;
    case RVInstr::ORI:
      return FSMState::EXORI;
    case RVInstr::XORI:
      return FSMState::EXXORI;
    case RVInstr::SLLI:
      return FSMState::EXSLLI;
    case RVInstr::SRLI:
      return FSMState::EXSRLI;
    case RVInstr::SRAI:
      return FSMState::EXSRAI;
    case RVInstr::LUI:
      return FSMState::EXLUI;
    case RVInstr::AUIPC:
      return FSMState::EXAUIP;
    case RVInstr::MUL:
      return FSMState::EXMUL;
    case RVInstr::MULH:
      return FSMState::EXMULH;
    case RVInstr::MULHSU:
      return FSMState::EXMULHSU;
    case RVInstr::MULHU:
      return FSMState::EXMULHU;
    case RVInstr::DIV:
      return FSMState::EXDIV;
    case RVInstr::DIVU:
      return FSMState::EXDIVU;
    case RVInstr::REM:
      return FSMState::EXREM;
    case RVInstr::REMU:
      return FSMState::EXREMU;
    case RVInstr::MULW:
      return FSMState::EXMULW;
    case RVInstr::DIVW:
      return FSMState::EXDIVW;
    case RVInstr::DIVUW:
      return FSMState::EXDIVUW;
    case RVInstr::REMW:
      return FSMState::EXREMW;
    case RVInstr::REMUW:
      return FSMState::EXREMUW;
    case RVInstr::ADDW:
      return FSMState::EXADDW;
    case RVInstr::SUBW:
      return FSMState::EXSUBW;
    case RVInstr::SLLW:
      return FSMState::EXSLLW;
    case RVInstr::SRLW:
      return FSMState::EXSRLW;
    case RVInstr::SRLIW:
      return FSMState::EXSRLIW;
    case RVInstr::SRAW:
      return FSMState::EXSRAW;
    case RVInstr::ADDIW:
      return FSMState::EXADDIW;
    case RVInstr::SLLIW:
      return FSMState::EXSLLIW;
    case RVInstr::SRAIW:
      return FSMState::EXSRAIW;
    case RVInstr::LB: case RVInstr::LH:
    case RVInstr::LW: case RVInstr::LBU:
    case RVInstr::LHU: case RVInstr::LWU:
    case RVInstr::LD:
    case RVInstr::SB: case RVInstr::SH:
    case RVInstr::SW: case RVInstr::SD:
      return FSMState::EXMEMOP;
    case RVInstr::ECALL:
      return FSMState::EXECALL;
    default:
      return FSMState::EX;
    }
  }

  static FSMState do_MEM_stateFromOPC(RVInstr opc) {
    switch (opc) {
    case RVInstr::LB:
      return FSMState::MEMLB;
    case RVInstr::LH:
      return FSMState::MEMLH;
    case RVInstr::LW:
      return FSMState::MEMLW;
    case RVInstr::LBU:
      return FSMState::MEMLBU;
    case RVInstr::LHU:
      return FSMState::MEMLHU;
    case RVInstr::LWU:
      return FSMState::MEMLWU;
    case RVInstr::LD:
      return FSMState::MEMLD;
    case RVInstr::SB:
      return FSMState::MEMSB;
    case RVInstr::SH:
      return FSMState::MEMSH;
    case RVInstr::SW:
      return FSMState::MEMSW;
    case RVInstr::SD:
      return FSMState::MEMSD;
    default:
      return FSMState::MEM;
    }
  }

  static FSMState do_statePort(RVInstr opc, FSMState state) {
    switch(state) {
    case FSMState::INVALID:
      return FSMState::INVALID;
    case FSMState::IF:
      return FSMState::ID;
    case FSMState::ID:
      return do_EX_stateFromOPC(opc);
    case FSMState::EX:
      return FSMState::MEM;
    case FSMState::EXJALR:
      return FSMState::MEMJALR;
    case FSMState::EXADD: case FSMState::EXSUB:
    case FSMState::EXSLT: case FSMState::EXSLTU:
    case FSMState::EXAND: case FSMState::EXOR:
    case FSMState::EXXOR: case FSMState::EXSLL:
    case FSMState::EXSRL: case FSMState::EXSRA:
    case FSMState::EXADDI: case FSMState::EXSLTI:
    case FSMState::EXSLTIU: case FSMState::EXANDI:
    case FSMState::EXORI: case FSMState::EXXORI:
    case FSMState::EXSLLI: case FSMState::EXSRLI:
    case FSMState::EXSRAI: case FSMState::EXLUI:
    case FSMState::EXMUL: case FSMState::EXMULH:
    case FSMState::EXMULHSU: case FSMState::EXMULHU:
    case FSMState::EXDIV: case FSMState::EXDIVU:
    case FSMState::EXREM: case FSMState::EXREMU:
    case FSMState::EXMULW: case FSMState::EXDIVW:
    case FSMState::EXDIVUW: case FSMState::EXREMW:
    case FSMState::EXREMUW: case FSMState::EXADDW:
    case FSMState::EXSUBW: case FSMState::EXSRLW:
    case FSMState::EXSRLIW: case FSMState::EXSLLW:
    case FSMState::EXSRAW: case FSMState::EXADDIW:
    case FSMState::EXSLLIW: case FSMState::EXSRAIW:
      return FSMState::MEMALINS;
    case FSMState::EXMEMOP:
      return do_MEM_stateFromOPC(opc);
    case FSMState::MEM:
      return FSMState::WB;
    case FSMState::MEMLB: case FSMState::MEMLH:
    case FSMState::MEMLW: case FSMState::MEMLBU:
    case FSMState::MEMLHU: case FSMState::MEMLWU:
    case FSMState::MEMLD:
      return FSMState::WBMEMLOAD;
    case FSMState::WB: case FSMState::EXJAL: case FSMState::MEMJALR:
    case FSMState::EXCJE: case FSMState::EXCJNE: case FSMState::EXCJGE:
    case FSMState::EXCJLT: case FSMState::EXCJLTU: case FSMState::EXCJGEU:
    case FSMState::MEMALINS: case FSMState::WBMEMLOAD: case FSMState::EXECALL:
    case FSMState::MEMSB: case FSMState::MEMSH: case FSMState::MEMSW:
    case FSMState::MEMSD: case FSMState::EXAUIP:
      return FSMState::IF;
    }
    return FSMState::IF;
  }

  RVInstr getCurrentOpcode() const {
    return opcode.eValue<RVInstr>();
  }

  FSMState getCurrentState() const {
    return stateRegCtr->out.eValue<FSMState>();
  }

  void addState(FSMState s, StateSignals o, TransitionFunc t) {
    auto idx = static_cast<int>(s);
    states.at(idx) = {o, t};
  }

  const StateInfo& getStateInfo(FSMState s) const {
    auto idx = static_cast<int>(s);
    return states[idx];
  }

  const StateInfo& getCurrentStateInfo() const {
    return getStateInfo(getCurrentState());
  }

  FSMState getNextState() const {
    return getCurrentStateInfo().transitions(getCurrentOpcode());
  }

public:
  RVMCControl(const std::string &name, SimComponent *parent)
  : Component(name, parent) {
    assert (states.empty());
    states.resize(magic_enum::enum_count<FSMState>());

#define to(x) [](RVInstr i){ ((void)i); return FSMState::x; }

    //States common to all operations
    addState(FSMState::IF, {
        .ir_write = true,
        .pc_write = true,
        .pc_src = PcSrc::PC4,
        .alu_op1_src = AluSrc1::PC,
        .alu_op2_src = AluSrc2::INPC,
        .alu_control = ALUControl::ADD,
        .mem_read = true,
    }, to(ID));

    addState(FSMState::ID, {
        .alu_op1_src = AluSrc1::PCOLD,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::ADD,
      }, do_EX_stateFromOPC); // TODO

    //Generic execution state
    addState(FSMState::EX, {

      }, to(MEM));

    //Specific States by Execution Type
    //Jump operations
    addState(FSMState::EXJAL, {
        .pc_write = true,
        .pc_src = PcSrc::ALU,
        .reg_write = true,
        .reg_src = RegWrSrc::PC4,
      }, to(IF));
    addState(FSMState::EXJALR, {
        // TODO: no .write_reg = true, nor .write_pc = true,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::ADD,
      }, to(MEMJALR));

    //Branches
    addState(FSMState::EXCJE, {
        .branch = true,
        .pc_src = PcSrc::ALU,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::SUB,
      }, to(IF));
    addState(FSMState::EXCJNE, {
        .branch = true,
        .pc_src = PcSrc::ALU,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::SUB,
      }, to(IF));
    addState(FSMState::EXCJGE, {
        .branch = true,
        .pc_src = PcSrc::ALU,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::SUB,
      }, to(IF));
    addState(FSMState::EXCJLT, {
        .branch = true,
        .pc_src = PcSrc::ALU,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::SUB,
      }, to(IF));
    addState(FSMState::EXCJLTU, {
        .branch = true,
        .pc_src = PcSrc::ALU,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::SUB,
      }, to(IF));
    addState(FSMState::EXCJGEU, {
        .branch = true,
        .pc_src = PcSrc::ALU,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::SUB,
      }, to(IF));

    //R-Type
    //Arimethic
    addState(FSMState::EXADD, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSUB, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSLT, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSLTU, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXAND, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXOR, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXXOR, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSLL, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSRL, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSRA, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXMUL, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXMULH, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXMULHSU, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXMULHU, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXDIV, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXDIVU, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXREM, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXREMU, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXMULW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXDIVW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXDIVUW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXREMW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXREMUW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXADDW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSUBW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSLLW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSRLW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSRAW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));

    //I-Type
    //Arimethic
    addState(FSMState::EXADDI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSLTI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSLTIU, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXANDI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXORI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXXORI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSLLI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSRLI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSRAI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXLUI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXADDIW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSLLIW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSRLIW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));
    addState(FSMState::EXSRAIW, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(MEMALINS));

    //Ecall
    addState(FSMState::EXECALL, {
        .ecall = true,
      }, to(IF));

    //Memory load
    addState(FSMState::EXMEMOP, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::ADD,
      }, do_MEM_stateFromOPC); // TODO

    //Generic memory state
    addState(FSMState::MEM, {

      }, to(WB));

    //Memory states
    addState(FSMState::MEMJALR, { // TODO:rename to WBJALR, or use generic WB
        .pc_write = true,
        .pc_src = PcSrc::ALU,
        .reg_write = true,
        .reg_src = RegWrSrc::PC4,
      }, to(IF));

    //I-Type
    //Memory Load
    addState(FSMState::MEMLB, {
        .mem_read = true,
        .mem_op = MemOp::LB,
      }, to(WBMEMLOAD));
    addState(FSMState::MEMLH, {
        .mem_read = true,
        .mem_op = MemOp::LH,
      }, to(WBMEMLOAD));
    addState(FSMState::MEMLW, {
        .mem_read = true,
        .mem_op = MemOp::LW,
      }, to(WBMEMLOAD));
    addState(FSMState::MEMLBU, {
        .mem_read = true,
        .mem_op = MemOp::LBU,
      }, to(WBMEMLOAD));
    addState(FSMState::MEMLHU, {
        .mem_read = true,
        .mem_op = MemOp::LHU,
      }, to(WBMEMLOAD));
    addState(FSMState::MEMLWU, {
        .mem_read = true,
        .mem_op = MemOp::LWU,
      }, to(WBMEMLOAD));
    addState(FSMState::MEMLD, {
        .mem_read = true,
        .mem_op = MemOp::LD,
      }, to(WBMEMLOAD));

    //Memory Store
    addState(FSMState::MEMSB, {
        .mem_write = true,
        .mem_op = MemOp::SB,
      }, to(IF));
    addState(FSMState::MEMSH, {
        .mem_write = true,
        .mem_op = MemOp::SH,
      }, to(IF));
    addState(FSMState::MEMSW, {
        .mem_write = true,
        .mem_op = MemOp::SW,
      }, to(IF));
    addState(FSMState::MEMSD, {
        .mem_write = true,
        .mem_op = MemOp::SD,
      }, to(IF));

    //Generic Write Back state
    addState(FSMState::WB, {

      }, to(IF));

    //Specific Write Back states
    //R-Type
    //Arimethic
    addState(FSMState::MEMALINS, { // TODO: rename to WBALU
        .reg_write = true,
        .reg_src = RegWrSrc::ALURES,
      }, to(IF));

    //I-Type
    //Memory Load
    addState(FSMState::WBMEMLOAD, {
        .reg_write = true,
        .reg_src = RegWrSrc::MEMREAD,
      }, to(IF));

    addState(FSMState::EXAUIP, { // TODO: RENAME WBAUIP
        .reg_write = true,
        .reg_src = RegWrSrc::ALURES,
      }, to(IF));

    addState(FSMState::INVALID, {}, to(INVALID));
#undef to

    ir_write << [=] { return getCurrentStateInfo().outs.ir_write; };
    pc_write << [=] { return getCurrentStateInfo().outs.pc_write; };
    branch << [=] { return getCurrentStateInfo().outs.branch; };
    pc_src << [=] { return getCurrentStateInfo().outs.pc_src; };
    reg_write << [=] { return getCurrentStateInfo().outs.reg_write; };
    reg_src << [=] { return getCurrentStateInfo().outs.reg_src; };
    alu_op1_src << [=] { return getCurrentStateInfo().outs.alu_op1_src; };
    alu_op2_src << [=] { return getCurrentStateInfo().outs.alu_op2_src; };
    alu_ctrl << [=] {
      auto c = getCurrentStateInfo().outs.alu_control;
      return c == ALUControl::INSTRUCTION_DEPENDENT ? do_alu_ctrl(getCurrentOpcode())
      : c == ALUControl::ADD ? ALUOp::ADD
      : (assert(c == ALUControl::SUB), ALUOp::SUB);
    };
    mem_read << [=] { return getCurrentStateInfo().outs.mem_read; };
    mem_write << [=] { return getCurrentStateInfo().outs.mem_write; };

    mem_ctrl << [=] { return getCurrentStateInfo().outs.mem_op; };
    comp_ctrl << [=] { return do_comp_ctrl(getCurrentOpcode()); };

    // ecall signal is inverted
    ecall << [=] { return !getCurrentStateInfo().outs.ecall; };

    stateOutPort << [=] { return getNextState(); };

    stateOutPort >> stateRegCtr->in;
  }

  bool inFirstState() const {
    return getCurrentState() == FSMState::IF;
  }

  bool inLastState() const {
    return getNextState() == FSMState::IF;
  }

  ///Subcomponents

  SUBCOMPONENT(stateRegCtr, Register<enumBitWidth<FSMState>()>);

  ///Ports

  /// May be useful for debug, should be hidden
  OUTPUTPORT_ENUM(stateOutPort,FSMState);

  INPUTPORT_ENUM(opcode, RVInstr);
  OUTPUTPORT(ir_write, 1);
  OUTPUTPORT(pc_write, 1);
  OUTPUTPORT(branch, 1);
  OUTPUTPORT_ENUM(pc_src,PcSrc);
  OUTPUTPORT(reg_write, 1);
  OUTPUTPORT_ENUM(reg_src, RegWrSrc);
  OUTPUTPORT_ENUM(alu_op1_src, AluSrc1);
  OUTPUTPORT_ENUM(alu_op2_src, AluSrc2);
  OUTPUTPORT_ENUM(alu_ctrl, ALUOp);
  OUTPUTPORT(mem_read,1);
  OUTPUTPORT(mem_write, 1);
  OUTPUTPORT_ENUM(mem_ctrl, MemOp);
  OUTPUTPORT_ENUM(comp_ctrl, CompOp);
  OUTPUTPORT(ecall,1);
};

} // namespace core
} // namespace vsrtl
