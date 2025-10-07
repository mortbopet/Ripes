#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "../../ripesvsrtlprocessor.h"

#include "processors/RISC-V/riscv.h"
#include "processors/RISC-V/rv_alu.h"
#include "processors/RISC-V/rv_branch.h"
#include "processors/RISC-V/rv_control.h"
#include "processors/RISC-V/rv_decode.h"
#include "processors/RISC-V/rv_ecallchecker.h"
#include "processors/RISC-V/rv_immediate.h"
#include "processors/RISC-V/rv_memory.h"

// Shared dual-issue components
#include "../rv6s_dual/rv6s_dual_common.h"
#include "../rv6s_dual/rv6s_dual_control.h"
#include "../rv6s_dual/rv6s_dual_instr_mem.h"
#include "../rv6s_dual/rv6s_dual_uncompress.h"

// Specialized VLIW components
#include "rv5s_vliw_common.h"
#include "rv5s_vliw_control.h"
#include "rv5s_vliw_forwardingunit.h"
#include "rv5s_vliw_registerfile.h"

// Stage separating registers
#include "rv5s_vliw_exmem.h"
#include "rv5s_vliw_idex.h"
#include "rv5s_vliw_ifid.h"
#include "rv5s_vliw_memwb.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RV5S_VLIW : public RipesVSRTLProcessor {
  static_assert(std::is_same<uint32_t, XLEN_T>::value ||
                    std::is_same<uint64_t, XLEN_T>::value,
                "Only supports 32- and 64-bit variants");
  static constexpr unsigned XLEN = sizeof(XLEN_T) * CHAR_BIT;

public:
  enum Lane { EXEC, DATA, LANECOUNT };
  enum Stage { IF, ID, EX, MEM, WB, STAGECOUNT };
  RV5S_VLIW(const QStringList &extensions)
      : RipesVSRTLProcessor("5-Stage static dual-issue VLIW RISC-V Processor") {
    m_enabledISA = ISAInfoRegistry::getISA<XLenToRVISA<XLEN>()>(extensions);
    decode_exec->setISA(m_enabledISA);
    decode_data->setISA(m_enabledISA);
    uncompress_dual->setISA(m_enabledISA);

    /* clang-format off */
    // -----------------------------------------------------------------------
    // IF
    // --- Instruction memory ---
    instr_mem->setMemory(m_memory);
    pc_reg->out >> instr_mem->addr;
    
    // --- Decompression ---
    instr_mem->data_out  >> uncompress_dual->instr1;
    instr_mem->data_out2 >> uncompress_dual->instr2;
    
    // --- Program counter ---
    2 >> pc_inc1->get(PcInc::INC2);
    4 >> pc_inc1->get(PcInc::INC4);
    uncompress_dual->Pc_Inc1 >> pc_inc1->select;
    
    2 >> pc_inc2->get(PcInc::INC2);
    4 >> pc_inc2->get(PcInc::INC4);
    uncompress_dual->Pc_Inc2 >> pc_inc2->select;
    
    pc_inc1->out >> pc_inc_sum->op1;
    pc_inc2->out >> pc_inc_sum->op2;
    
    pc_inc_sum->out >> pc_8->op1;
    pc_reg->out >> pc_8->op2;
    
    pc_8->out >> pc_src->get(PcSrc::PC4);
    
    pc_src->out >> pc_reg->in;

    // --- Controlflow - syscall ---
    // Note: pc_src works uses the PcSrc enum, but is selected by the boolean
    // signal from the controlflow OR gate. PcSrc enum values must adhere to the
    // boolean 0/1 values.
    controlflow_or->out >> pc_src->select;

    controlflow_or->out >> *efsc_or->in[0];
    ecallChecker->syscallExit >> *efsc_or->in[1];
    

    // --- IF/ID ---
    pc_8->out >> ifid_reg->pc8_in;
    pc_reg->out >> ifid_reg->pc_in;
    pc_inc1->out >> ifid_reg->pc_data_offset_in;
    uncompress_dual->exp_instr1 >> ifid_reg->instr1_in;
    uncompress_dual->exp_instr2 >> ifid_reg->instr2_in;
    1 >> ifid_reg->enable;
    efsc_or->out >> ifid_reg->clear;
    1 >> ifid_reg->valid_in; // Always valid unless register is cleared
    

    // -----------------------------------------------------------------------
    // ID
    // --- Decode ---
    ifid_reg->instr1_out >> decode_exec->instr;
    ifid_reg->instr2_out >> decode_data->instr;

    // --- Immediate ---
    decode_exec->opcode >> imm_exec->opcode;
    ifid_reg->instr1_out >> imm_exec->instr;

    decode_data->opcode >> imm_data->opcode;
    ifid_reg->instr2_out >> imm_data->instr;

    // --- Registers ---
    registerFile->setMemory(m_regMem);
    
    /*exec*/
    decode_exec->r1_reg_idx >> registerFile->r1_1_addr;
    decode_exec->r2_reg_idx >> registerFile->r2_1_addr;
    reg_wr_src_exec->out >> registerFile->data_1_in;
    memwb_reg->wr_reg_idx_exec_out >> registerFile->wr_1_addr;
    memwb_reg->reg_do_write_exec_out >> registerFile->wr_1_en;
    
    /*data*/
    decode_data->r1_reg_idx >> registerFile->r1_2_addr;
    decode_data->r2_reg_idx >> registerFile->r2_2_addr;
    memwb_reg->mem_read_out >> registerFile->data_2_in;
    memwb_reg->wr_reg_idx_data_out >> registerFile->wr_2_addr;
    memwb_reg->reg_do_write_data_out >> registerFile->wr_2_en;
    
    // --- Control signals ---
    decode_exec->opcode >> control->opcode_exec;
    decode_data->opcode >> control->opcode_data;

    // --- Ecall checker ---
    decode_exec->opcode >> ecallChecker->opcode;
    ecallChecker->setSyscallCallback(&trapHandler);
    0 >> ecallChecker->stallEcallHandling;
    
    // --- ID/EX ---
    1 >> idex_reg->enable;
    controlflow_or->out >> idex_reg->clear;

    /*pc*/
    ifid_reg->pc8_out >> idex_reg->pc8_in;
    ifid_reg->pc_out >> idex_reg->pc_in;
    ifid_reg->pc_data_offset_out >> idex_reg->pc_data_offset_in;

    /*exec*/
    registerFile->r1_1_out >> idex_reg->r1_exec_in;
    registerFile->r2_1_out >> idex_reg->r2_exec_in;
    imm_exec->imm >> idex_reg->imm_exec_in;
    decode_exec->r1_reg_idx >> idex_reg->rd_reg1_idx_exec_in;
    decode_exec->r2_reg_idx >> idex_reg->rd_reg2_idx_exec_in;
    decode_exec->wr_reg_idx >> idex_reg->wr_reg_idx_exec_in;
    control->reg_do_write_exec_ctrl >> idex_reg->reg_do_write_exec_in;
    
    /*data*/
    registerFile->r1_2_out >> idex_reg->r1_data_in;
    registerFile->r2_2_out >> idex_reg->r2_data_in;
    imm_data->imm >> idex_reg->imm_data_in;
    decode_data->r1_reg_idx >> idex_reg->rd_reg1_idx_data_in;
    decode_data->r2_reg_idx >> idex_reg->rd_reg2_idx_data_in;
    decode_data->wr_reg_idx >> idex_reg->wr_reg_idx_data_in;
    control->reg_do_write_data_ctrl >> idex_reg->reg_do_write_data_in;
    
    /*control*/
    control->reg_wr_src_exec_ctrl >> idex_reg->reg_wr_src_exec_ctrl_in;
    control->alu_exec_op1_ctrl >> idex_reg->alu_exec_op1_ctrl_in;
    control->alu_exec_op2_ctrl >> idex_reg->alu_exec_op2_ctrl_in;
    control->alu_exec_ctrl >> idex_reg->alu_exec_ctrl_in;
    
    control->mem_do_write_ctrl >> idex_reg->mem_do_write_in;
    control->mem_ctrl >> idex_reg->mem_op_in;
    control->comp_ctrl >> idex_reg->br_op_in;
    control->do_branch >> idex_reg->do_br_in;
    control->do_jump >> idex_reg->do_jmp_in;
    
    control->exec_is_valid >> idex_reg->exec_is_valid_in;
    control->data_is_valid >> idex_reg->data_is_valid_in;

    ifid_reg->valid_out >> idex_reg->valid_in;


    // -----------------------------------------------------------------------
    // EX
    // --- ALU ---
    /*exec*/
    alu_exec_op1_src->out >> alu_exec->op1;
    alu_exec_op2_src->out >> alu_exec->op2;
    idex_reg->alu_exec_ctrl_out >> alu_exec->ctrl;
    
    exec_reg1_fw_src->out >> alu_exec_op1_src->get(AluSrc1::REG1);
    idex_reg->pc_out      >> alu_exec_op1_src->get(AluSrc1::PC);
    idex_reg->alu_exec_op1_ctrl_out >> alu_exec_op1_src->select;

    exec_reg2_fw_src->out  >> alu_exec_op2_src->get(AluSrc2::REG2);
    idex_reg->imm_exec_out >> alu_exec_op2_src->get(AluSrc2::IMM);
    idex_reg->alu_exec_op2_ctrl_out >> alu_exec_op2_src->select;
    
    /*data*/
    data_reg1_fw_src->out  >> alu_data->op1;
    idex_reg->imm_data_out >> alu_data->op2;
    static_cast<VSRTL_VT_U>(ALUOp::ADD) >> alu_data->ctrl;
    // could be replaced by simple add unit but for symmetry & visual reasons we chose an ALU

    // --- Forwarding ---
    /*fw-exec-reg1*/
    idex_reg->r1_exec_out        >> exec_reg1_fw_src->get(ForwardSrcVliw::IdStage);
    exmem_reg->alu_exec_res_out  >> exec_reg1_fw_src->get(ForwardSrcVliw::MemStageExec);
    reg_wr_src_exec->out         >> exec_reg1_fw_src->get(ForwardSrcVliw::WbStageExec);
    memwb_reg->mem_read_out      >> exec_reg1_fw_src->get(ForwardSrcVliw::WbStageData);
    funit->alu_exec_reg1_fw_ctrl >> exec_reg1_fw_src->select;
    
    /*fw-exec-reg2*/
    idex_reg->r2_exec_out        >> exec_reg2_fw_src->get(ForwardSrcVliw::IdStage);
    exmem_reg->alu_exec_res_out  >> exec_reg2_fw_src->get(ForwardSrcVliw::MemStageExec);
    reg_wr_src_exec->out         >> exec_reg2_fw_src->get(ForwardSrcVliw::WbStageExec);
    memwb_reg->mem_read_out      >> exec_reg2_fw_src->get(ForwardSrcVliw::WbStageData);
    funit->alu_exec_reg2_fw_ctrl >> exec_reg2_fw_src->select;
    
    /*fw-data-reg1*/
    idex_reg->r1_data_out        >> data_reg1_fw_src->get(ForwardSrcVliw::IdStage);
    exmem_reg->alu_exec_res_out  >> data_reg1_fw_src->get(ForwardSrcVliw::MemStageExec);
    reg_wr_src_exec->out         >> data_reg1_fw_src->get(ForwardSrcVliw::WbStageExec);
    memwb_reg->mem_read_out      >> data_reg1_fw_src->get(ForwardSrcVliw::WbStageData);
    funit->alu_data_reg1_fw_ctrl >> data_reg1_fw_src->select;
    
    /*fw--mem-data*/
    idex_reg->r2_data_out        >> mem_data_fw_src->get(ForwardSrcVliw::IdStage);
    exmem_reg->alu_exec_res_out  >> mem_data_fw_src->get(ForwardSrcVliw::MemStageExec);
    reg_wr_src_exec->out         >> mem_data_fw_src->get(ForwardSrcVliw::WbStageExec);
    memwb_reg->mem_read_out      >> mem_data_fw_src->get(ForwardSrcVliw::WbStageData);
    funit->mem_data_fw_ctrl      >> mem_data_fw_src->select;

    /*fw-unit*/
    idex_reg->rd_reg1_idx_exec_out >> funit->id_reg1_idx_exec;
    idex_reg->rd_reg2_idx_exec_out >> funit->id_reg2_idx_exec;
    
    idex_reg->rd_reg1_idx_data_out >> funit->id_reg1_idx_data;
    idex_reg->rd_reg2_idx_data_out >> funit->id_reg2_idx_data;
    
    exmem_reg->wr_reg_idx_exec_out >> funit->mem_reg_wr_idx_exec;
    exmem_reg->reg_do_write_exec_out >> funit->mem_reg_wr_en_exec;

    memwb_reg->wr_reg_idx_exec_out >> funit->wb_reg_wr_idx_exec;
    memwb_reg->reg_do_write_exec_out >> funit->wb_reg_wr_en_exec;

    memwb_reg->wr_reg_idx_data_out >> funit->wb_reg_wr_idx_data;
    memwb_reg->reg_do_write_data_out >> funit->wb_reg_wr_en_data;


    // --- Branch ---
    idex_reg->br_op_out >> branch->comp_op;
    exec_reg1_fw_src->out >> branch->op1;
    exec_reg2_fw_src->out >> branch->op2;

    branch->res >> *br_and->in[0];
    idex_reg->do_br_out >> *br_and->in[1];
    br_and->out >> *controlflow_or->in[0];
    idex_reg->do_jmp_out >> *controlflow_or->in[1];

    alu_exec->res >> pc_src->get(PcSrc::ALU);

    // --- EX/MEM ---
    0 >> exmem_reg->clear;
    1 >> exmem_reg->enable;
    idex_reg->valid_out >> exmem_reg->valid_in;
    
    /*pc*/
    idex_reg->pc_out >> exmem_reg->pc_in;
    idex_reg->pc8_out >> exmem_reg->pc8_in;
    idex_reg->pc_data_offset_out >> exmem_reg->pc_data_offset_in;
    
    /*exec*/
    alu_exec->res >> exmem_reg->alu_exec_res_in;
    idex_reg->wr_reg_idx_exec_out >> exmem_reg->wr_reg_idx_exec_in;
    idex_reg->reg_do_write_exec_out >> exmem_reg->reg_do_write_exec_in;
    
    /*data*/
    alu_data->res >> exmem_reg->alu_data_res_in;
    idex_reg->wr_reg_idx_data_out >> exmem_reg->wr_reg_idx_data_in;
    idex_reg->reg_do_write_data_out >> exmem_reg->reg_do_write_data_in;
    mem_data_fw_src->out >> exmem_reg->r2_data_in;
    
    /*control*/
    idex_reg->reg_wr_src_exec_ctrl_out >> exmem_reg->reg_wr_src_exec_ctrl_in;
    idex_reg->mem_do_write_out >> exmem_reg->mem_do_write_in;
    idex_reg->exec_is_valid_out >> exmem_reg->exec_is_valid_in;
    idex_reg->data_is_valid_out >> exmem_reg->data_is_valid_in;
    idex_reg->mem_op_out >> exmem_reg->mem_op_in;

    // -----------------------------------------------------------------------
    // MEM
    // --- Memory ---
    exmem_reg->alu_data_res_out >> data_mem->addr;
    exmem_reg->mem_do_write_out >> data_mem->wr_en;
    exmem_reg->r2_data_out >> data_mem->data_in;
    exmem_reg->mem_op_out >> data_mem->op;
    data_mem->mem->setMemory(m_memory);

    // --- MEM/WB ---
    exmem_reg->pc_out >> memwb_reg->pc_in;
    exmem_reg->pc8_out >> memwb_reg->pc8_in;
    exmem_reg->pc_data_offset_out >> memwb_reg->pc_data_offset_in;

    exmem_reg->alu_exec_res_out >> memwb_reg->alu_exec_res_in;
    data_mem->data_out >> memwb_reg->mem_read_in;

    exmem_reg->reg_wr_src_exec_ctrl_out >> memwb_reg->reg_wr_src_exec_ctrl_in;
    exmem_reg->wr_reg_idx_exec_out >> memwb_reg->wr_reg_idx_exec_in;
    exmem_reg->reg_do_write_exec_out >> memwb_reg->reg_do_write_exec_in;

    exmem_reg->wr_reg_idx_data_out >> memwb_reg->wr_reg_idx_data_in;
    exmem_reg->reg_do_write_data_out >> memwb_reg->reg_do_write_data_in;

    exmem_reg->exec_is_valid_out >> memwb_reg->exec_is_valid_in;
    exmem_reg->data_is_valid_out >> memwb_reg->data_is_valid_in;
    exmem_reg->mem_op_out >> memwb_reg->mem_op_in;

    exmem_reg->valid_out >> memwb_reg->valid_in;
    
    // -----------------------------------------------------------------------
    // WB
    memwb_reg->alu_exec_res_out >> reg_wr_src_exec->get(RegWrExecSrc::ALURES);
    memwb_reg->pc8_out >> reg_wr_src_exec->get(RegWrExecSrc::PC8);
    memwb_reg->reg_wr_src_exec_ctrl_out >> reg_wr_src_exec->select;
    /* clang-format on */
  }

  // Design subcomponents
  SUBCOMPONENT(registerFile, TYPE(RegisterFile_VLIW<XLEN>));
  SUBCOMPONENT(alu_exec, TYPE(ALU<XLEN>));
  SUBCOMPONENT(alu_data, TYPE(ALU<XLEN>));
  SUBCOMPONENT(control, Control_VLIW);
  SUBCOMPONENT(imm_exec, TYPE(Immediate<XLEN>));
  SUBCOMPONENT(imm_data, TYPE(Immediate<XLEN>));
  SUBCOMPONENT(decode_exec, TYPE(Decode<XLEN>));
  SUBCOMPONENT(decode_data, TYPE(Decode<XLEN>));
  SUBCOMPONENT(branch, TYPE(Branch<XLEN>));
  SUBCOMPONENT(pc_8, Adder<XLEN>);
  SUBCOMPONENT(uncompress_dual, TYPE(UncompressDual<XLEN>));
  SUBCOMPONENT(pc_inc_sum, Adder<XLEN>);

  // Registers
  SUBCOMPONENT(pc_reg, Register<XLEN>);

  // Stage seperating registers
  SUBCOMPONENT(ifid_reg, TYPE(IFID_VLIW<XLEN>));
  SUBCOMPONENT(idex_reg, TYPE(IDEX_VLIW<XLEN>));
  SUBCOMPONENT(exmem_reg, TYPE(EXMEM_VLIW<XLEN>));
  SUBCOMPONENT(memwb_reg, TYPE(MEMWB_VLIW<XLEN>));

  // Multiplexers
  SUBCOMPONENT(reg_wr_src_exec, TYPE(EnumMultiplexer<RegWrExecSrc, XLEN>));
  SUBCOMPONENT(pc_src, TYPE(EnumMultiplexer<PcSrc, XLEN>));
  SUBCOMPONENT(alu_exec_op1_src, TYPE(EnumMultiplexer<AluSrc1, XLEN>));
  SUBCOMPONENT(alu_exec_op2_src, TYPE(EnumMultiplexer<AluSrc2, XLEN>));

  SUBCOMPONENT(exec_reg1_fw_src, TYPE(EnumMultiplexer<ForwardSrcVliw, XLEN>));
  SUBCOMPONENT(exec_reg2_fw_src, TYPE(EnumMultiplexer<ForwardSrcVliw, XLEN>));
  SUBCOMPONENT(data_reg1_fw_src, TYPE(EnumMultiplexer<ForwardSrcVliw, XLEN>));
  SUBCOMPONENT(mem_data_fw_src, TYPE(EnumMultiplexer<ForwardSrcVliw, XLEN>));

  SUBCOMPONENT(pc_inc1, TYPE(EnumMultiplexer<PcInc, XLEN>));
  SUBCOMPONENT(pc_inc2, TYPE(EnumMultiplexer<PcInc, XLEN>));

  // Memories
  SUBCOMPONENT(instr_mem, TYPE(ROM_DUAL<XLEN, c_RVInstrWidth>));
  SUBCOMPONENT(data_mem, TYPE(RVMemory<XLEN, XLEN>));

  // Forwarding unit
  SUBCOMPONENT(funit, ForwardingUnit_VLIW);

  // Gates
  // True if branch instruction and branch taken
  SUBCOMPONENT(br_and, TYPE(And<1, 2>));
  // True if branch taken or jump instruction
  SUBCOMPONENT(controlflow_or, TYPE(Or<1, 2>));
  // True if controlflow action or performing syscall finishing
  SUBCOMPONENT(efsc_or, TYPE(Or<1, 2>));

  // Address spaces
  ADDRESSSPACEMM(m_memory);
  ADDRESSSPACE(m_regMem);

  SUBCOMPONENT(ecallChecker, EcallChecker);

private:
  bool get_lane_is_valid(StageIndex stage) const {
    /* clang-format off */
    switch (stage.lane()) {
      case EXEC:
        switch( stage.index() ) {
          case ID:  return control->exec_is_valid.uValue();
          case EX:  return idex_reg->exec_is_valid_out.uValue();
          case MEM: return exmem_reg->exec_is_valid_out.uValue();
          case WB:  return memwb_reg->exec_is_valid_out.uValue();
          default: case IF: return false;
        } break;
      
      case DATA:
        switch( stage.index() ) {
          case ID:  return control->data_is_valid.uValue();
          case EX:  return idex_reg->data_is_valid_out.uValue();
          case MEM: return exmem_reg->data_is_valid_out.uValue();
          case WB:  return memwb_reg->data_is_valid_out.uValue();
          default: case IF: return false;
        } break;
      
      default: Q_UNREACHABLE();
    }
    /* clang-format on */
  }

public:
  // Ripes interface compliance
  const ProcessorStructure &structure() const override { return m_structure; }
  unsigned int getPcForStage(StageIndex idx) const override {
    Q_ASSERT(idx.lane() == EXEC || idx.lane() == DATA);

    int pc = 0;
    /* clang-format off */
    switch(idx.index()) {
      case IF:  pc = pc_reg->out.uValue(); break;
      case ID:  pc = ifid_reg->pc_out.uValue(); break;
      case EX:  pc = idex_reg->pc_out.uValue(); break;
      case MEM: pc = exmem_reg->pc_out.uValue(); break;
      case WB:  pc = memwb_reg->pc_out.uValue(); break;
      default:  break;
    }

    if (idx.lane()==DATA) {
      switch(idx.index()) {
        case IF:  pc += pc_inc1->out.uValue(); break;
        case ID:  pc += ifid_reg->pc_data_offset_out.uValue(); break;
        case EX:  pc += idex_reg->pc_data_offset_out.uValue(); break;
        case MEM: pc += exmem_reg->pc_data_offset_out.uValue(); break;
        case WB:  pc += memwb_reg->pc_data_offset_out.uValue(); break;
        default:  break;
      }
    }
    /* clang-format on */

    return pc;
  }
  AInt nextFetchedAddress() const override { return pc_src->out.uValue(); }
  QString stageName(StageIndex idx) const override {
    Q_ASSERT(idx.lane() == EXEC || idx.lane() == DATA);

    /* clang-format off */
    if (idx.second == IF)  return idx.lane() == EXEC ? "IF1"  : "IF2";
    if (idx.second == ID)  return idx.lane() == EXEC ? "ID1"  : "ID2";
    if (idx.second == EX)  return idx.lane() == EXEC ? "EX1"  : "EX2";
    if (idx.second == MEM) return idx.lane() == EXEC ? "MEM1" : "MEM2";
    if (idx.second == WB)  return idx.lane() == EXEC ? "WB1"  : "WB2";
    /* clang-format on */
    Q_UNREACHABLE();
  }
  StageInfo stageInfo(StageIndex stage) const override {
    bool stageValid = true;
    // Has the pipeline stage been filled?
    stageValid &= stage.index() <= m_cycleCount;

    // clang-format off
    // Has the stage been cleared?
    switch(stage.index()){
      case ID:  stageValid &= ifid_reg->valid_out.uValue(); break;
      case EX:  stageValid &= idex_reg->valid_out.uValue(); break;
      case MEM: stageValid &= exmem_reg->valid_out.uValue(); break;
      case WB:  stageValid &= memwb_reg->valid_out.uValue(); break;
      default: case IF: break;
    }

    // Is the stage carrying a valid (executable) PC?
    stageValid &= isExecutableAddress( getPcForStage(stage) );
    
    // Are we currently clearing the pipeline due to a syscall exit?
    // if such, all stages before the EX stage are invalid
    if(stage.index() < EX){
      stageValid &= !ecallChecker->isSysCallExiting();
    }
    
    #define STAGE_STATE(sid, sr, mr)                            \
      if (m_cycleCount >= (sid)) {                              \
        if ((sr)->valid_out.uValue() == 0) {                    \
          state = StageInfo::State::Flushed;                    \
        } else if( stageValid && !get_lane_is_valid(stage) ) {  \
          state = StageInfo::State::Invalid;                    \
        }                                                       \
      }

    // Gather stage state info
    StageInfo::State state = StageInfo::State::None;
    switch (stage.index()) {
      case ID:  STAGE_STATE( ID , ifid_reg, control->mem_ctrl ); break;
      case EX:  STAGE_STATE( EX , idex_reg, idex_reg->mem_op_out ); break;
      case MEM: STAGE_STATE( MEM, exmem_reg, exmem_reg->mem_op_out ); break;
      case WB:  STAGE_STATE( WB , memwb_reg, memwb_reg->mem_op_out ); break;
      default: case IF: break;
    }
    #undef STAGE_STATE
    // clang-format on

    return StageInfo({getPcForStage(stage), stageValid, state});
  }

  void setProgramCounter(AInt address) override {
    pc_reg->forceValue(0, address);
    propagateDesign();
  }
  void setPCInitialValue(AInt address) override {
    pc_reg->setInitValue(address);
  }
  AddressSpaceMM &getMemory() override { return *m_memory; }
  VInt getRegister(const std::string_view &, unsigned i) const override {
    return registerFile->getRegister(i);
  }
  void finalize(FinalizeReason fr) override {
    if ((fr & FinalizeReason::exitSyscall) &&
        !ecallChecker->isSysCallExiting()) {
      // An exit system call was executed. Record the cycle of the execution,
      // and enable the ecallChecker's system call exiting signal.
      m_syscallExitCycle = m_cycleCount;
    }
    ecallChecker->setSysCallExiting(ecallChecker->isSysCallExiting() ||
                                    (fr & FinalizeReason::exitSyscall));
  }
  const std::vector<StageIndex> breakpointTriggeringStages() const override {
    return {{DATA, IF}, {EXEC, IF}};
  }

  MemoryAccess dataMemAccess() const override {
    return memToAccessInfo(data_mem);
  }
  MemoryAccess instrMemAccess() const override {
    auto instrAccess = memToAccessInfo(instr_mem);
    instrAccess.type = MemoryAccess::Read;
    return instrAccess;
  }

  bool finished() const override {
    // The processor is finished when there are no more valid instructions in
    // the pipeline
    bool allStagesInvalid = true;

    for (int lane = 0; lane < LANECOUNT; lane++) {
      for (int stage = IF; stage < STAGECOUNT; stage++) {
        allStagesInvalid &= !stageInfo({lane, stage}).stage_valid;
        if (!allStagesInvalid)
          break;
      }
    }
    return allStagesInvalid;
  }

  void setRegister(const std::string_view &, unsigned i, VInt v) override {
    setSynchronousValue(registerFile->rf_1->_wr_mem, i, v);
  }

  void clockProcessor() override {
    // An instruction has been retired if the instruction in the WB stage is
    // valid and the PC is within the executable range of the program
    if (memwb_reg->valid_out.uValue() != 0 &&
        isExecutableAddress(memwb_reg->pc_out.uValue())) {
      m_instructionsRetired += 2;
    }

    Design::clock();
  }

  void reverse() override {
    if (m_syscallExitCycle != -1 && m_cycleCount == m_syscallExitCycle) {
      // We are about to undo an exit syscall instruction. In this case, the
      // syscall exiting sequence should be terminate
      ecallChecker->setSysCallExiting(false);
      m_syscallExitCycle = -1;
    }
    Design::reverse();
    m_instructionsRetired -= 2;
  }

  void reset() override {
    ecallChecker->setSysCallExiting(false);
    Design::reset();
    m_syscallExitCycle = -1;
  }

  static ProcessorISAInfo supportsISA() { return RVISA::supportsISA<XLEN>(); }
  std::shared_ptr<ISAInfoBase> implementsISA() const override {
    return m_enabledISA;
  }
  std::shared_ptr<const ISAInfoBase> fullISA() const override {
    return RVISA::fullISA<XLEN>();
  }

  const std::set<std::string_view> registerFiles() const override {
    std::set<std::string_view> rfs;
    rfs.insert(RVISA::GPR);

    if (implementsISA()->extensionEnabled("F")) {
      rfs.insert(RVISA::FPR);
    }
    return rfs;
  }

private:
  /**
   * @brief m_syscallExitCycle
   * The variable will contain the cycle of which an exit system call was
   * executed. From this, we may determine when we roll back an exit system
   * call during rewinding.
   */
  long long m_syscallExitCycle = -1;
  std::shared_ptr<ISAInfoBase> m_enabledISA;
  ProcessorStructure m_structure = {{0, 5}, {1, 5}};
};

} // namespace core
} // namespace vsrtl
