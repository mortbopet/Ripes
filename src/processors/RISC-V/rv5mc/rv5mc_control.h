#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"
#include "processors/RISC-V/riscv.h"


namespace vsrtl {
namespace core {
using namespace Ripes;

enum class AluSrc2MC { REG2, IMM, INPC };
enum class PCOutscr { PCOld, PC };
enum class PCOldInscr { PCout, PCin };

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
};

class RVMCControl : public Component {
public:
  /* clang-format off */

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

  static VSRTL_VT_U do_branch_ctrl(FSMState state) {
    switch (state) {
    case FSMState::EXCJE: case FSMState::EXCJNE: case FSMState::EXCJGE:
    case FSMState::EXCJLT: case FSMState::EXCJLTU: case FSMState::EXCJGEU:
      return 1;
    default:
      return 0;

    }
  }

  static VSRTL_VT_U do_write_pc_ctrl(FSMState state) {

    switch (state) {
    case FSMState::IF: case FSMState::EXJAL: case FSMState::MEMJALR:
      return 1;
    default:
      return 0;
    }
  }

  static MemOp do_mem_ctrl(FSMState state) {
    switch (state) {
    case FSMState::MEMLB:
      return MemOp::LB;
    case FSMState::MEMLH:
      return MemOp::LH;
    case FSMState::MEMLW:
      return MemOp::LW;
    case FSMState::MEMLBU:
      return MemOp::LBU;
    case FSMState::MEMLHU:
      return MemOp::LHU;
    case FSMState::MEMLWU:
      return MemOp::LWU;
    case FSMState::MEMLD:
      return MemOp::LD;
    case FSMState::MEMSB:
      return MemOp::SB;
    case FSMState::MEMSH:
      return MemOp::SH;
    case FSMState::MEMSW:
      return MemOp::SW;
    case FSMState::MEMSD:
      return MemOp::SD;
    default:
      return MemOp::NOP;
    }
  }

  static VSRTL_VT_U do_reg_do_write_ctrl(FSMState state) {
    switch (state) {
    case FSMState::EXJAL: case FSMState::MEMJALR:
    case FSMState::MEMALINS: case FSMState::EXAUIP:
    case FSMState::WBMEMLOAD:
      return 1;
    default:
      return 0;
    }
  }

  static RegWrSrc do_reg_wr_src_ctrl(FSMState state) {
    switch (state) {
    case FSMState::WBMEMLOAD:
      return RegWrSrc::MEMREAD;
    case FSMState::EXJAL: case FSMState::MEMJALR:
      return RegWrSrc::PC4;
    case FSMState::MEMALINS:
    case FSMState::EXAUIP:
    default:
      return RegWrSrc::ALURES;
    }
  }

  static AluSrc1 do_alu_op1_ctrl(FSMState state) {
    switch (state) {
    case FSMState::IF: case FSMState::ID:
      return AluSrc1::PC;
    case FSMState::EXJALR: case FSMState::EXCJE: case FSMState::EXCJNE: case FSMState::EXCJGE:
    case FSMState::EXCJLT: case FSMState::EXCJLTU: case FSMState::EXCJGEU: case FSMState::EXADD:
    case FSMState::EXSUB: case FSMState::EXSLT: case FSMState::EXSLTU: case FSMState::EXAND:
    case FSMState::EXOR: case FSMState::EXXOR: case FSMState::EXSLL: case FSMState::EXSRL:
    case FSMState::EXSRA: case FSMState::EXADDI: case FSMState::EXSLTI: case FSMState::EXSLTIU:
    case FSMState::EXANDI: case FSMState::EXORI: case FSMState::EXXORI: case FSMState::EXSLLI:
    case FSMState::EXSRLI: case FSMState::EXSRAI: case FSMState::EXLUI: case FSMState::EXMUL:
    case FSMState::EXMULH: case FSMState::EXMULHSU: case FSMState::EXMULHU: case FSMState::EXDIV:
    case FSMState::EXDIVU: case FSMState::EXREM: case FSMState::EXREMU: case FSMState::EXMULW:
    case FSMState::EXDIVW: case FSMState::EXDIVUW: case FSMState::EXREMW: case FSMState::EXREMUW:
case FSMState::EXADDW: case FSMState::EXSUBW: case FSMState::EXSLLW: case FSMState::EXSRLW:
case FSMState::EXSRLIW: case FSMState::EXSRAW:
    case FSMState::EXADDIW: case FSMState::EXSLLIW: case FSMState::EXSRAIW: case FSMState::EXMEMOP:
    default:
      return AluSrc1::REG1;
    }
  }

  static AluSrc2MC do_alu_op2_ctrl(FSMState state) {
    switch (state) {
    case FSMState::IF:
      return AluSrc2MC::INPC;
    case FSMState::ID: case FSMState::EXJALR: case FSMState::EXADDI:
    case FSMState::EXSLTI: case FSMState::EXSLTIU: case FSMState::EXANDI:
    case FSMState::EXORI: case FSMState::EXXORI: case FSMState::EXSLLI:
    case FSMState::EXSRLI: case FSMState::EXSRAI: case FSMState::EXLUI:
    case FSMState::EXADDIW: case FSMState::EXSLLIW: case FSMState::EXSRAIW:
    case FSMState::EXMEMOP: case FSMState::EXSRLIW:
      return AluSrc2MC::IMM;
    case FSMState::EXCJE: case FSMState::EXCJNE: case FSMState::EXCJGE:
    case FSMState::EXCJLT: case FSMState::EXCJLTU: case FSMState::EXCJGEU:
    case FSMState::EXADD: case FSMState::EXSUB: case FSMState::EXSLT:
    case FSMState::EXSLTU: case FSMState::EXAND: case FSMState::EXOR:
    case FSMState::EXXOR: case FSMState::EXSLL: case FSMState::EXSRL:
    case FSMState::EXSRA: case FSMState::EXMUL: case FSMState::EXMULH:
    case FSMState::EXMULHSU: case FSMState::EXMULHU: case FSMState::EXDIV:
    case FSMState::EXDIVU: case FSMState::EXREM: case FSMState::EXREMU:
    case FSMState::EXMULW: case FSMState::EXDIVW: case FSMState::EXDIVUW:
    case FSMState::EXREMW: case FSMState::EXREMUW: case FSMState::EXADDW:
case FSMState::EXSUBW: case FSMState::EXSLLW: case FSMState::EXSRLW:
    case FSMState::EXSRAW:
    default:
      return AluSrc2MC::REG2;
    }
  }

  static ALUOp do_alu_ctrl(FSMState state) {
    switch (state) {
    case FSMState::IF: case FSMState::ID: case FSMState::EXJALR: case FSMState::EXADD:
    case FSMState::EXADDI: case FSMState::EXMEMOP:
      return ALUOp::ADD;
    case FSMState::EXCJE: case FSMState::EXCJNE: case FSMState::EXCJGE: case FSMState::EXCJLT:
    case FSMState::EXCJLTU: case FSMState::EXCJGEU: case FSMState::EXSUB:
      return ALUOp::SUB;
    case FSMState::EXSLT: case FSMState::EXSLTI:
      return ALUOp::LT;
    case FSMState::EXSLTU: case FSMState::EXSLTIU:
      return ALUOp::LTU;
    case FSMState::EXAND: case FSMState::EXANDI:
      return ALUOp::AND;
    case FSMState::EXOR: case FSMState::EXORI:
      return ALUOp::OR;
    case FSMState::EXXOR: case FSMState::EXXORI:
      return ALUOp::XOR;
    case FSMState::EXSLL: case FSMState::EXSLLI:
      return ALUOp::SL;
    case FSMState::EXSRL: case FSMState::EXSRLI:
      return ALUOp::SRL;
    case FSMState::EXSRA: case FSMState::EXSRAI:
      return ALUOp::SRA;
    case FSMState::EXLUI:
      return ALUOp::LUI;
    case FSMState::EXMUL:
      return ALUOp::MUL;
    case FSMState::EXMULH:
      return ALUOp::MULH;
    case FSMState::EXMULHSU:
      return ALUOp::MULHSU;
    case FSMState::EXMULHU:
      return ALUOp::MULHU;
    case FSMState::EXDIV:
      return ALUOp::DIV;
    case FSMState::EXDIVU:
      return ALUOp::DIVU;
    case FSMState::EXREM:
      return ALUOp::REM;
    case FSMState::EXREMU:
      return ALUOp::REMU;
    case FSMState::EXMULW:
      return ALUOp::MULW;
    case FSMState::EXDIVW:
      return ALUOp::DIVW;
    case FSMState::EXDIVUW:
      return ALUOp::DIVUW;
    case FSMState::EXREMW:
      return ALUOp::REMW;
    case FSMState::EXREMUW:
      return ALUOp::REMUW;
    case FSMState::EXADDW:
      return ALUOp::ADDW;
    case FSMState::EXSUBW:
      return ALUOp::SUBW;
    case FSMState::EXSRLW:
    case FSMState::EXSRLIW:
      return ALUOp::SRLW;
case FSMState::EXSLLW:
return ALUOp::SLW;
    case FSMState::EXSRAW:
      return ALUOp::SRAW;
    case FSMState::EXADDIW:
      return ALUOp::ADDW;
    case FSMState::EXSLLIW:
      return ALUOp::SLW;
    case FSMState::EXSRAIW:
      return ALUOp::SRAW;
    default: return ALUOp::NOP;
    }
  }

  static VSRTL_VT_U do_do_mem_write_ctrl(FSMState opc) {
    switch(opc) {
    case FSMState::MEMSB: case FSMState::MEMSH:
    case FSMState::MEMSW: case FSMState::MEMSD:
      return 1;
    default: return 0;
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

  static VSRTL_VT_U do_RIWrite(FSMState state) {
    switch (state) {
    case FSMState::IF:
      return 1;
    default:
      return 0;
    }
  }

  static VSRTL_VT_U do_pc_scr_ctrl(FSMState state) {
    switch (state) {
    case FSMState::EXJAL: case FSMState::MEMJALR: case FSMState::EXCJE: case FSMState::EXCJNE:
    case FSMState::EXCJGE: case FSMState::EXCJLT: case FSMState::EXCJLTU: case FSMState::EXCJGEU:
      return PcSrc::ALU;
    case FSMState::IF: default:
      return PcSrc::PC4;
    }
  }

  static VSRTL_VT_U do_pc_old_w_ctrl (FSMState state) {
    switch (state) {
    case FSMState::EXJAL: case FSMState::MEMJALR:
    case FSMState::EXCJE: case FSMState::EXCJNE:
    case FSMState::EXCJGE: case FSMState::EXCJLT:
    case FSMState::EXCJLTU: case FSMState::EXCJGEU:
    case FSMState::EXAUIP: case FSMState::MEMALINS:
    case FSMState::MEMSB: case FSMState::MEMSH:
    case FSMState::MEMSW: case FSMState::MEMSD:
    case FSMState::WB: case FSMState::WBMEMLOAD:
    case FSMState::EXECALL:
      return 1;
    default:
      return 0;
    }
  }

  static PCOutscr do_pc_out_scr_ctrl (FSMState state) {
    switch (state) {
    case FSMState::IF: case FSMState::EXJAL: case FSMState::MEMJALR:
      return PCOutscr::PC;
    default:
      return PCOutscr::PCOld;
    }
  }

  static PCOldInscr do_pc_old_in_scr_ctrl (FSMState state) {
    switch (state) {
    case FSMState::EXJAL: case FSMState::MEMJALR:
      return PCOldInscr::PCin;
    default:
      return PCOldInscr::PCout;
    }
  }

  static VSRTL_VT_U do_read_men_ctrl (FSMState state) {
    switch (state) {
    case FSMState::MEMLB: case FSMState::MEMLH:
    case FSMState::MEMLW: case FSMState::MEMLBU:
    case FSMState::MEMLHU: case FSMState::MEMLWU:
    case FSMState::MEMLD:
      return 1;
    default:
      return 0;
    }
  }

  static VSRTL_VT_U do_ecall_ctr (FSMState state) {
    switch (state) {
    case FSMState::EXECALL:
      return 0;
    default:
      return 1;
    }
  }

  bool do_finish_this_cycle(){
    switch (stateRegCtr->out.eValue<FSMState>()){
    case FSMState::EXJAL: case FSMState::MEMJALR:
    case FSMState::EXCJE: case FSMState::EXCJNE:
    case FSMState::EXCJGE: case FSMState::EXCJLT:
    case FSMState::EXCJLTU: case FSMState::EXCJGEU:
    case FSMState::EXAUIP: case FSMState::MEMALINS:
    case FSMState::MEMSB: case FSMState::MEMSH:
    case FSMState::MEMSW: case FSMState::MEMSD:
    case FSMState::WB: case FSMState::WBMEMLOAD:
    case FSMState::EXECALL:
      return true;
    default:
      return false;
    }
  }
  /* clang-format on */

public:
  RVMCControl(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    comp_ctrl << [=] { return do_comp_ctrl(opcode.eValue<RVInstr>()); };
    do_branch << [=] { return do_branch_ctrl( stateIntPort.eValue<FSMState>()); };
    do_write_pc << [=] { return do_write_pc_ctrl( stateIntPort.eValue<FSMState>()); };
    mem_ctrl << [=] { return do_mem_ctrl(stateIntPort.eValue<FSMState>()); };
    reg_do_write_ctrl << [=] { return do_reg_do_write_ctrl(stateIntPort.eValue<FSMState>()); };
    reg_wr_src_ctrl << [=] { return do_reg_wr_src_ctrl(stateIntPort.eValue<FSMState>()); };
    alu_op1_ctrl << [=] { return do_alu_op1_ctrl(stateIntPort.eValue<FSMState>()); };
    alu_op2_ctrl << [=] { return do_alu_op2_ctrl( stateIntPort.eValue<FSMState>()); };
    alu_ctrl << [=] { return do_alu_ctrl( stateIntPort.eValue<FSMState>()); };
    mem_do_write_ctrl << [=] { return do_do_mem_write_ctrl(stateIntPort.eValue<FSMState>()); };

    RIWrite << [=] { return do_RIWrite( stateIntPort.eValue<FSMState>());};

    pc_scr_ctrl << [=] { return do_pc_scr_ctrl( stateIntPort.eValue<FSMState>());};
    pc_old_w << [=] {return do_pc_old_w_ctrl( stateIntPort.eValue<FSMState>());};
    pc_out_scr_crtl << [=] {return do_pc_out_scr_ctrl( stateIntPort.eValue<FSMState>());};

    pc_old_in_scr_ctrl << [=] {return do_pc_old_in_scr_ctrl( stateIntPort.eValue<FSMState>());};

    stateOutPort << [=] { return do_statePort( opcode.eValue<RVInstr>(), stateIntPort.eValue<FSMState>());};

    read_men_ctrl << [=] {return do_read_men_ctrl( stateIntPort.eValue<FSMState>());};

    ecall_ctr << [=] {return do_ecall_ctr(stateIntPort.eValue<FSMState>());};

    stateOutPort >> stateRegCtr->in;

    stateRegCtr->out >> stateIntPort;

  }

  ///Subcomponents

  SUBCOMPONENT(stateRegCtr, Register<enumBitWidth<FSMState>()>);

  ///Ports

  ///Internals

  OUTPUTPORT_ENUM(stateOutPort,FSMState);
  INPUTPORT_ENUM(stateIntPort,FSMState);

  ///Externals

  INPUTPORT_ENUM(opcode, RVInstr);

  OUTPUTPORT(RIWrite, 1);
  OUTPUTPORT(pc_old_w,1);

  OUTPUTPORT(ecall_ctr,1);

  OUTPUTPORT_ENUM(pc_scr_ctrl,PcSrc);
  OUTPUTPORT_ENUM(pc_out_scr_crtl,PCOutscr);
  OUTPUTPORT_ENUM(pc_old_in_scr_ctrl,PCOldInscr);

  OUTPUTPORT(reg_do_write_ctrl, 1);
  OUTPUTPORT(mem_do_write_ctrl, 1);
  OUTPUTPORT(do_branch, 1);
  OUTPUTPORT(do_write_pc, 1);
  OUTPUTPORT(read_men_ctrl,1);
  OUTPUTPORT_ENUM(comp_ctrl, CompOp);
  OUTPUTPORT_ENUM(reg_wr_src_ctrl, RegWrSrc);
  OUTPUTPORT_ENUM(mem_ctrl, MemOp);
  OUTPUTPORT_ENUM(alu_op1_ctrl, AluSrc1);
  OUTPUTPORT_ENUM(alu_op2_ctrl, AluSrc2MC);
  OUTPUTPORT_ENUM(alu_ctrl, ALUOp);


};

} // namespace core
} // namespace vsrtl
