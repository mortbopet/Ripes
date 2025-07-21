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
  enum class AluSrc2 { REG2, IMM, PC_INC };
  enum class ALUControl { ADD, SUB, INSTRUCTION_DEPENDENT };

  enum class FSMState {
    IF,
    ID,

    EX, //Generic execution state, used for determining the Stage in stageInfo

    EXJALR,
    EXBRANCH,
    EXALUR,
    EXALUI,
    EXMEMOP,
    EXECALL,

    MEM, //Generic memory state
    
    WBJ,
    MEMLOAD,
    MEMSTORE,

    WB, //Generic writeback state

    WBALU,
    WBMEMLOAD,
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
    bool mem_write = false;
    bool ecall = false;
  };
  typedef FSMState (*TransitionFunc)(RVInstr);

  struct StateInfo { // There is an instance of this struct for each valid FSMState, created at initialization
    StateSignals outs; // Values for VSRT signals
    TransitionFunc transitions;
  };

class RVMCControl : public Component {
public:
  /* clang-format off */
  std::vector<StateInfo> states; // Information about valid states, indexed by name (FSMState)

  static CompOp do_comp_ctrl(RVInstr opc) {
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

  static ALUOp do_alu_ctrl(RVInstr opc) {
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

  static MemOp do_mem_ctrl(RVInstr opc) {
    switch(opc){
      case RVInstr::SB: return MemOp::SB;
      case RVInstr::SH: return MemOp::SH;
      case RVInstr::SW: return MemOp::SW;
      case RVInstr::SD: return MemOp::SD;
      case RVInstr::LB: return MemOp::LB;
      case RVInstr::LH: return MemOp::LH;
      case RVInstr::LW: return MemOp::LW;
      case RVInstr::LD: return MemOp::LD;
      case RVInstr::LBU: return MemOp::LBU;
      case RVInstr::LHU: return MemOp::LHU;
      case RVInstr::LWU: return MemOp::LWU;
      default:
        return MemOp::NOP;
    }
  }

  RVInstr getCurrentOpcode() const {
    return opcode.eValue<RVInstr>();
  }

  FSMState getCurrentState() const {
    return state_reg->out.eValue<FSMState>();
  }

  void setInitialState() const {
    state_reg->forceValue(0, magic_enum::enum_integer(FSMState::IF));
  }

  // Add a new state to the vector of valid states, indicating the
  // name, signals and transition fuction for the state.
  // This function needs to be called once for each state at initialization time.
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

#define to(x) [](RVInstr){ return FSMState::x; }

    //States common to all operations
    addState(FSMState::IF, {
        .ir_write = true,
        .pc_write = true,
        .pc_src = PcSrc::PC4,
        .alu_op1_src = AluSrc1::PC,
        .alu_op2_src = AluSrc2::PC_INC,
        .alu_control = ALUControl::ADD,
    }, to(ID));

    addState(FSMState::ID, {
        .alu_op1_src = AluSrc1::PCOLD,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::ADD,
      }, [](RVInstr i){
        switch (i) {
        case RVInstr::JAL:
          return FSMState::WBJ;
        case RVInstr::JALR:
          return FSMState::EXJALR;
        case RVInstr::BEQ: case RVInstr::BNE: case RVInstr::BLT: case RVInstr::BGE: case RVInstr::BLTU: case RVInstr::BGEU:
          return FSMState::EXBRANCH;
        case RVInstr::ADD: case RVInstr::SUB: case RVInstr::SLT: case RVInstr::SLTU: case RVInstr::AND: case RVInstr::OR:
        case RVInstr::XOR: case RVInstr::SLL: case RVInstr::SRL: case RVInstr::SRA: case RVInstr::MUL: case RVInstr::MULH:
        case RVInstr::MULHSU: case RVInstr::MULHU: case RVInstr::DIV: case RVInstr::DIVU: case RVInstr::REM: case RVInstr::REMU:
        case RVInstr::MULW: case RVInstr::DIVW: case RVInstr::DIVUW: case RVInstr::REMW: case RVInstr::REMUW: case RVInstr::ADDW:
        case RVInstr::SUBW: case RVInstr::SLLW: case RVInstr::SRLW: case RVInstr::SRAW:
          return FSMState::EXALUR;
        case RVInstr::ADDI: case RVInstr::SLTI: case RVInstr::SLTIU: case RVInstr::ANDI: case RVInstr::ORI: case RVInstr::XORI:
        case RVInstr::SLLI: case RVInstr::SRLI: case RVInstr::SRAI: case RVInstr::LUI: case RVInstr::SRLIW: case RVInstr::ADDIW:
        case RVInstr::SLLIW: case RVInstr::SRAIW:
          return FSMState::EXALUI;
        case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU: case RVInstr::LHU: case RVInstr::LWU:
        case RVInstr::LD: case RVInstr::SB: case RVInstr::SH: case RVInstr::SW: case RVInstr::SD:
          return FSMState::EXMEMOP;
        case RVInstr::ECALL:
          return FSMState::EXECALL;
        case RVInstr::AUIPC:
          return FSMState::WBALU;
        default:
          // Illegal instruction
          return FSMState::IF; // skip. TODO: exception
      }});

    //Specific States by Execution Type
    //Jump operations
    addState(FSMState::EXJALR, {
        // TODO: no .write_reg = true, nor .write_pc = true,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::ADD,
      }, to(WBJ));

    //Branches
    addState(FSMState::EXBRANCH, {
        .branch = true,
        .pc_src = PcSrc::ALU,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::SUB,
      }, to(IF));

    //R-Type
    //Arimethic
    addState(FSMState::EXALUR, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(WBALU));

    //I-Type
    //Arimethic
    addState(FSMState::EXALUI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(WBALU));

    //Memory load
    addState(FSMState::EXMEMOP, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::ADD,
    }, [](RVInstr i){
      switch (i) {
        case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LD:
        case RVInstr::LBU: case RVInstr::LHU: case RVInstr::LWU:
          return FSMState::MEMLOAD;
        case RVInstr::SB: case RVInstr::SH: case RVInstr::SW: case RVInstr::SD:
          return FSMState::MEMSTORE;
        default:
          assert(false);
      }
    });

    //Ecall
    addState(FSMState::EXECALL, {
      .ecall = true,
    }, to(IF));

    //Memory states
    addState(FSMState::MEMLOAD, {
        //.mem_read = true, // the memory reads every cycle from the addres in ALU_out
      }, to(WBMEMLOAD));

    addState(FSMState::MEMSTORE, {
        .mem_write = true,
      }, to(IF));

    //Writeback states
    addState(FSMState::WBALU, {
        .reg_write = true,
        .reg_src = RegWrSrc::ALURES,
      }, to(IF));

    addState(FSMState::WBMEMLOAD, {
        .reg_write = true,
        .reg_src = RegWrSrc::MEMREAD,
      }, to(IF));

    addState(FSMState::WBJ, {
        .pc_write = true,
        .pc_src = PcSrc::ALU,
        .reg_write = true,
        .reg_src = RegWrSrc::PC4,
      }, to(IF));

#undef to

    setInitialState();

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
    mem_write << [=] { return getCurrentStateInfo().outs.mem_write; };

    mem_ctrl << [=] { return do_mem_ctrl(getCurrentOpcode());};
    comp_ctrl << [=] { return do_comp_ctrl(getCurrentOpcode()); };

    // ecall signal is inverted
    ecall << [=] { return !getCurrentStateInfo().outs.ecall; };

    next_state << [=] { return getNextState(); };
    current_state << [=] { return getCurrentState(); };

    next_state >> state_reg->in;
  }

  bool inFirstState() const {
    return getCurrentState() == FSMState::IF;
  }

  bool inLastState() const {
    return getNextState() == FSMState::IF;
  }

  ///Subcomponents

  SUBCOMPONENT(state_reg, Register<enumBitWidth<FSMState>()>);

  ///Ports

  /// May be useful for debug, should be hidden
  OUTPUTPORT_ENUM(next_state,FSMState);
  OUTPUTPORT_ENUM(current_state,FSMState);

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
  OUTPUTPORT(mem_write, 1);
  OUTPUTPORT_ENUM(mem_ctrl, MemOp);
  OUTPUTPORT_ENUM(comp_ctrl, CompOp);
  OUTPUTPORT(ecall,1);
};

}
} // namespace core
} // namespace vsrtl
