#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_comparator.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "../../branch/branchpredictionprocessor.h"
#include "../../ripesvsrtlprocessor.h"

// Functional units
#include "../riscv.h"
#include "../rv_alu.h"
#include "../rv_branch.h"
#include "../rv_control.h"
#include "../rv_decode.h"
#include "../rv_ecallchecker.h"
#include "../rv_immediate.h"
#include "../rv_memory.h"
#include "../rv_registerfile.h"
#include "../rv_uncompress.h"

// Stage separating registers
#include "../rv5s/rv5s_exmem.h"
#include "../rv5s/rv5s_idex.h"
#include "../rv5s/rv5s_memwb.h"
#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_ifid.h"

// Forwarding & Hazard detection unit
#include "../rv5s/rv5s_forwardingunit.h"
#include "../rv5s/rv5s_hazardunit.h"

// Branch prediction unit
#include "../../branch/predictorhandler.h"
#include "rv5s_branch_miss.h"
#include "rv5s_branch_nextpc.h"
#include "rv5s_branch_regs.h"
#include "rv5s_branchunit.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RV5S_BR : public RipesVSRTLProcessor, public BranchPredictionProcessor {
  static_assert(std::is_same<uint32_t, XLEN_T>::value ||
                    std::is_same<uint64_t, XLEN_T>::value,
                "Only supports 32- and 64-bit variants");
  static constexpr unsigned XLEN = sizeof(XLEN_T) * CHAR_BIT;

public:
  enum Stage { IF = 0, ID = 1, EX = 2, MEM = 3, WB = 4, STAGECOUNT };
  RV5S_BR(const QStringList &extensions)
      : RipesVSRTLProcessor("5-Stage RISC-V Processor") {
    m_enabledISA = std::make_shared<ISAInfo<XLenToRVISA<XLEN>()>>(extensions);
    decode->setISA(m_enabledISA);
    uncompress->setISA(m_enabledISA);

    // -----------------------------------------------------------------------
    // Program counter
    pc_reg->out >> pc_4->op1;
    pc_inc->out >> pc_4->op2;
    br_nextpc->curr_next >> pc_reg->in;
    0 >> pc_reg->clear;
    br_pc_or->out >> pc_reg->enable;

    2 >> pc_inc->get(PcInc::INC2);
    4 >> pc_inc->get(PcInc::INC4);
    uncompress->Pc_Inc >> pc_inc->select;

    0 >> *efsc_or->in[0];
    ecallChecker->syscallExit >> *efsc_or->in[1];

    efsc_or->out >> *efschz_or->in[0];
    hzunit->hazardIDEXClear >> *efschz_or->in[1];

    // -----------------------------------------------------------------------
    // Instruction memory
    pc_reg->out >> instr_mem->addr;
    instr_mem->setMemory(m_memory);

    // -----------------------------------------------------------------------
    // Decode
    ifid_reg->instr_out >> decode->instr;

    // -----------------------------------------------------------------------
    // Control signals
    decode->opcode >> control->opcode;

    // -----------------------------------------------------------------------
    // Immediate
    decode->opcode >> immediate->opcode;
    ifid_reg->instr_out >> immediate->instr;

    // -----------------------------------------------------------------------
    // Registers
    decode->r1_reg_idx >> registerFile->r1_addr;
    decode->r2_reg_idx >> registerFile->r2_addr;
    reg_wr_src->out >> registerFile->data_in;

    memwb_reg->wr_reg_idx_out >> registerFile->wr_addr;
    memwb_reg->reg_do_write_out >> registerFile->wr_en;
    memwb_reg->mem_read_out >> reg_wr_src->get(RegWrSrc::MEMREAD);
    memwb_reg->alures_out >> reg_wr_src->get(RegWrSrc::ALURES);
    memwb_reg->pc4_out >> reg_wr_src->get(RegWrSrc::PC4);
    memwb_reg->reg_wr_src_ctrl_out >> reg_wr_src->select;

    registerFile->setMemory(m_regMem);

    // -----------------------------------------------------------------------
    // Branch
    idex_reg->br_op_out >> branch->comp_op;
    reg1_fw_src->out >> branch->op1;
    reg2_fw_src->out >> branch->op2;

    // -----------------------------------------------------------------------
    // ALU

    // Forwarding multiplexers
    idex_reg->r1_out >> reg1_fw_src->get(ForwardingSrc::IdStage);
    exmem_reg->alures_out >>
        reg1_fw_src->get(
            ForwardingSrc::MemStage); // Todo: Mem stage needs a mux to allow
                                      // for AUIPC forwarding
    reg_wr_src->out >> reg1_fw_src->get(ForwardingSrc::WbStage);
    funit->alu_reg1_forwarding_ctrl >> reg1_fw_src->select;

    idex_reg->r2_out >> reg2_fw_src->get(ForwardingSrc::IdStage);
    exmem_reg->alures_out >> reg2_fw_src->get(ForwardingSrc::MemStage);
    reg_wr_src->out >> reg2_fw_src->get(ForwardingSrc::WbStage);
    funit->alu_reg2_forwarding_ctrl >> reg2_fw_src->select;

    // ALU operand multiplexers
    reg1_fw_src->out >> alu_op1_src->get(AluSrc1::REG1);
    idex_reg->pc_out >> alu_op1_src->get(AluSrc1::PC);
    idex_reg->alu_op1_ctrl_out >> alu_op1_src->select;

    reg2_fw_src->out >> alu_op2_src->get(AluSrc2::REG2);
    idex_reg->imm_out >> alu_op2_src->get(AluSrc2::IMM);
    idex_reg->alu_op2_ctrl_out >> alu_op2_src->select;

    alu_op1_src->out >> alu->op1;
    alu_op2_src->out >> alu->op2;

    idex_reg->alu_ctrl_out >> alu->ctrl;

    // -----------------------------------------------------------------------
    // Data memory
    exmem_reg->alures_out >> data_mem->addr;
    exmem_reg->mem_do_write_out >> data_mem->wr_en;
    exmem_reg->r2_out >> data_mem->data_in;
    exmem_reg->mem_op_out >> data_mem->op;
    data_mem->mem->setMemory(m_memory);

    // -----------------------------------------------------------------------
    // Ecall checker

    idex_reg->opcode_out >> ecallChecker->opcode;
    ecallChecker->setSyscallCallback(&trapHandler);
    hzunit->stallEcallHandling >> ecallChecker->stallEcallHandling;

    // -----------------------------------------------------------------------
    // IF/ID
    pc_4->out >> ifid_reg->pc4_in;
    pc_reg->out >> ifid_reg->pc_in;
    uncompress->exp_instr >> ifid_reg->instr_in;
    hzunit->hazardFEEnable >> ifid_reg->enable;
    efsc_or->out >> *ifid_reg_clear_or->in[1];
    1 >> ifid_reg->valid_in; // Always valid unless register is cleared

    // -----------------------------------------------------------------------
    // Increment
    instr_mem->data_out >> uncompress->instr;

    // -----------------------------------------------------------------------
    // ID/EX
    hzunit->hazardIDEXEnable >> idex_reg->enable;
    hzunit->hazardIDEXClear >> idex_reg->stalled_in;
    efschz_or->out >> *idex_reg_clear_or->in[1];

    // Data
    ifid_reg->pc4_out >> idex_reg->pc4_in;
    ifid_reg->pc_out >> idex_reg->pc_in;
    registerFile->r1_out >> idex_reg->r1_in;
    registerFile->r2_out >> idex_reg->r2_in;
    immediate->imm >> idex_reg->imm_in;

    // Control
    decode->wr_reg_idx >> idex_reg->wr_reg_idx_in;
    control->reg_wr_src_ctrl >> idex_reg->reg_wr_src_ctrl_in;
    control->reg_do_write_ctrl >> idex_reg->reg_do_write_in;
    control->alu_op1_ctrl >> idex_reg->alu_op1_ctrl_in;
    control->alu_op2_ctrl >> idex_reg->alu_op2_ctrl_in;
    control->mem_do_write_ctrl >> idex_reg->mem_do_write_in;
    control->alu_ctrl >> idex_reg->alu_ctrl_in;
    control->mem_ctrl >> idex_reg->mem_op_in;
    control->comp_ctrl >> idex_reg->br_op_in;
    control->do_branch >> idex_reg->do_br_in;
    control->do_jump >> idex_reg->do_jmp_in;
    decode->r1_reg_idx >> idex_reg->rd_reg1_idx_in;
    decode->r2_reg_idx >> idex_reg->rd_reg2_idx_in;
    decode->opcode >> idex_reg->opcode_in;
    control->mem_do_read_ctrl >> idex_reg->mem_do_read_in;

    ifid_reg->valid_out >> idex_reg->valid_in;

    // -----------------------------------------------------------------------
    // EX/MEM
    1 >> exmem_reg->enable;
    hzunit->hazardEXMEMClear >> exmem_reg->clear;
    hzunit->hazardEXMEMClear >> *mem_stalled_or->in[0];
    idex_reg->stalled_out >> *mem_stalled_or->in[1];
    mem_stalled_or->out >> exmem_reg->stalled_in;

    // Data
    idex_reg->pc_out >> exmem_reg->pc_in;
    idex_reg->pc4_out >> exmem_reg->pc4_in;
    reg2_fw_src->out >> exmem_reg->r2_in;
    alu->res >> exmem_reg->alures_in;

    // Control
    idex_reg->reg_wr_src_ctrl_out >> exmem_reg->reg_wr_src_ctrl_in;
    idex_reg->wr_reg_idx_out >> exmem_reg->wr_reg_idx_in;
    idex_reg->reg_do_write_out >> exmem_reg->reg_do_write_in;
    idex_reg->mem_do_write_out >> exmem_reg->mem_do_write_in;
    idex_reg->mem_do_read_out >> exmem_reg->mem_do_read_in;
    idex_reg->mem_op_out >> exmem_reg->mem_op_in;

    idex_reg->valid_out >> exmem_reg->valid_in;

    // -----------------------------------------------------------------------
    // MEM/WB

    exmem_reg->stalled_out >> memwb_reg->stalled_in;

    // Data
    exmem_reg->pc_out >> memwb_reg->pc_in;
    exmem_reg->pc4_out >> memwb_reg->pc4_in;
    exmem_reg->alures_out >> memwb_reg->alures_in;
    data_mem->data_out >> memwb_reg->mem_read_in;

    // Control
    exmem_reg->reg_wr_src_ctrl_out >> memwb_reg->reg_wr_src_ctrl_in;
    exmem_reg->wr_reg_idx_out >> memwb_reg->wr_reg_idx_in;
    exmem_reg->reg_do_write_out >> memwb_reg->reg_do_write_in;

    exmem_reg->valid_out >> memwb_reg->valid_in;

    // -----------------------------------------------------------------------
    // Forwarding unit
    idex_reg->rd_reg1_idx_out >> funit->id_reg1_idx;
    idex_reg->rd_reg2_idx_out >> funit->id_reg2_idx;

    exmem_reg->wr_reg_idx_out >> funit->mem_reg_wr_idx;
    exmem_reg->reg_do_write_out >> funit->mem_reg_wr_en;

    memwb_reg->wr_reg_idx_out >> funit->wb_reg_wr_idx;
    memwb_reg->reg_do_write_out >> funit->wb_reg_wr_en;

    // -----------------------------------------------------------------------
    // Hazard detection unit
    decode->r1_reg_idx >> hzunit->id_reg1_idx;
    decode->r2_reg_idx >> hzunit->id_reg2_idx;

    idex_reg->mem_do_read_out >> hzunit->ex_do_mem_read_en;
    idex_reg->wr_reg_idx_out >> hzunit->ex_reg_wr_idx;

    exmem_reg->reg_do_write_out >> hzunit->mem_do_reg_write;

    memwb_reg->reg_do_write_out >> hzunit->wb_do_reg_write;

    idex_reg->opcode_out >> hzunit->opcode;

    // -----------------------------------------------------------------------
    // Branch prediction unit
    brunit->setProc(dynamic_cast<BranchPredictionProcessor *>(this));
    pc_reg->out >> brunit->curr_pc;

    brunit->curr_pre_targ >> br_ifid_reg->curr_pre_targ_in;
    brunit->curr_pre_take >> br_ifid_reg->curr_pre_take_in;

    br_ifid_reg->curr_pre_targ_out >> br_idex_reg->curr_pre_targ_in;
    br_ifid_reg->curr_pre_take_out >> br_idex_reg->curr_pre_take_in;

    idex_reg->pc_out >> brunit->prev_pc;
    br_idex_reg->curr_pre_take_out >> brunit->prev_pre_take;

    br_idex_reg->curr_pre_targ_out >> br_comp_target->op1;
    alu->res >> br_comp_target->op2;

    br_comp_target->out >> br_miss->curr_equal_targets;
    branch->res >> br_miss->curr_act_take;
    br_idex_reg->curr_pre_take_out >> br_miss->curr_pre_take;
    br_idex_reg->curr_is_b_out >> br_miss->curr_is_b;
    br_idex_reg->curr_is_j_out >> br_miss->curr_is_j;

    br_miss->curr_miss_1 >> *br_miss_or->in[0];
    br_miss->curr_miss_2 >> *br_miss_or->in[1];

    br_miss_or->out >> brunit->prev_pre_miss;

    br_idex_reg->curr_is_b_out >> brunit->prev_is_b;

    brunit->curr_pre_targ >> br_nextpc->curr_pre_targ;
    pc_4->out >> br_nextpc->pc_4_id;
    idex_reg->pc4_out >> br_nextpc->pc_4_ex;
    alu->res >> br_nextpc->curr_act_targ;
    brunit->curr_pre_take >> br_nextpc->curr_pre_take;
    br_miss->curr_miss_1 >> br_nextpc->curr_miss_1;
    br_miss->curr_miss_2 >> br_nextpc->curr_miss_2;

    br_miss_or->out >> *br_squash_not->in[0];

    brunit->curr_is_b >> *br_if_b_and->in[0];
    brunit->curr_is_j >> *br_if_j_and->in[0];
    hzunit->hazardFEEnable >> *br_if_b_and->in[1];
    hzunit->hazardFEEnable >> *br_if_j_and->in[1];
    br_squash_not->out >> *br_if_b_and->in[2];
    br_squash_not->out >> *br_if_j_and->in[2];
    br_if_b_and->out >> br_ifid_reg->curr_is_b_in;
    br_if_j_and->out >> br_ifid_reg->curr_is_j_in;

    br_ifid_reg->curr_is_b_out >> *br_id_b_and->in[0];
    br_ifid_reg->curr_is_j_out >> *br_id_j_and->in[0];
    br_squash_not->out >> *br_id_b_and->in[1];
    br_squash_not->out >> *br_id_j_and->in[1];
    br_id_b_and->out >> br_idex_reg->curr_is_b_in;
    br_id_j_and->out >> br_idex_reg->curr_is_j_in;

    hzunit->hazardFEEnable >> *br_pc_or->in[0];
    br_miss->curr_miss_1 >> *br_pc_or->in[1];
    br_miss->curr_miss_2 >> *br_pc_or->in[2];

    br_miss_or->out >> *ifid_reg_clear_or->in[0];
    ifid_reg_clear_or->out >> ifid_reg->clear;
    br_miss_or->out >> *idex_reg_clear_or->in[0];
    idex_reg_clear_or->out >> idex_reg->clear;

    efsc_or->out >> br_ifid_reg->clear;
    efschz_or->out >> br_idex_reg->clear;

    hzunit->hazardFEEnable >> br_ifid_reg->enable;
    hzunit->hazardIDEXEnable >> br_idex_reg->enable;
  }

  // Design subcomponents
  SUBCOMPONENT(registerFile, TYPE(RegisterFile<XLEN, true>));
  SUBCOMPONENT(alu, TYPE(ALU<XLEN>));
  SUBCOMPONENT(control, Control);
  SUBCOMPONENT(immediate, TYPE(Immediate<XLEN>));
  SUBCOMPONENT(decode, TYPE(Decode<XLEN>));
  SUBCOMPONENT(branch, TYPE(Branch<XLEN>));
  SUBCOMPONENT(pc_4, Adder<XLEN>);
  SUBCOMPONENT(uncompress, TYPE(Uncompress<XLEN>));

  // Registers
  SUBCOMPONENT(pc_reg, RegisterClEn<XLEN>);

  // Stage seperating registers
  SUBCOMPONENT(ifid_reg, TYPE(IFID<XLEN>));
  SUBCOMPONENT(idex_reg, TYPE(RV5S_IDEX<XLEN>));
  SUBCOMPONENT(exmem_reg, TYPE(RV5S_EXMEM<XLEN>));
  SUBCOMPONENT(memwb_reg, TYPE(RV5S_MEMWB<XLEN>));

  // Multiplexers
  SUBCOMPONENT(reg_wr_src, TYPE(EnumMultiplexer<RegWrSrc, XLEN>));
  SUBCOMPONENT(alu_op1_src, TYPE(EnumMultiplexer<AluSrc1, XLEN>));
  SUBCOMPONENT(alu_op2_src, TYPE(EnumMultiplexer<AluSrc2, XLEN>));
  SUBCOMPONENT(reg1_fw_src, TYPE(EnumMultiplexer<ForwardingSrc, XLEN>));
  SUBCOMPONENT(reg2_fw_src, TYPE(EnumMultiplexer<ForwardingSrc, XLEN>));
  SUBCOMPONENT(pc_inc, TYPE(EnumMultiplexer<PcInc, XLEN>));

  // Memories
  SUBCOMPONENT(instr_mem, TYPE(ROM<XLEN, c_RVInstrWidth>));
  SUBCOMPONENT(data_mem, TYPE(RVMemory<XLEN, XLEN>));

  // Forwarding & hazard detection units
  SUBCOMPONENT(funit, ForwardingUnit);
  SUBCOMPONENT(hzunit, HazardUnit);

  // Branch prediction unit and necessary gates/registers
  SUBCOMPONENT(brunit, TYPE(BranchUnit<XLEN_T, XLEN>));

  SUBCOMPONENT(br_ifid_reg, TYPE(RV5S_BR_IFID<XLEN>));
  SUBCOMPONENT(br_idex_reg, TYPE(RV5S_BR_IDEX<XLEN>));

  SUBCOMPONENT(br_comp_target, Eq<XLEN>);

  SUBCOMPONENT(br_miss, BranchMiss);

  SUBCOMPONENT(br_miss_or, TYPE(Or<1, 2>));
  SUBCOMPONENT(br_if_b_and, TYPE(And<1, 3>));
  SUBCOMPONENT(br_if_j_and, TYPE(And<1, 3>));
  SUBCOMPONENT(br_id_b_and, TYPE(And<1, 2>));
  SUBCOMPONENT(br_id_j_and, TYPE(And<1, 2>));
  SUBCOMPONENT(br_squash_not, TYPE(Not<1, 1>));
  SUBCOMPONENT(br_pc_or, TYPE(Or<1, 3>));

  SUBCOMPONENT(br_nextpc, BranchNextPC<XLEN>);

  SUBCOMPONENT(ifid_reg_clear_or, TYPE(Or<1, 2>));
  SUBCOMPONENT(idex_reg_clear_or, TYPE(Or<1, 2>));

  // Gates
  // True if controlflow action or performing syscall finishing
  SUBCOMPONENT(efsc_or, TYPE(Or<1, 2>));
  // True if above or stalling due to load-use hazard
  SUBCOMPONENT(efschz_or, TYPE(Or<1, 2>));

  SUBCOMPONENT(mem_stalled_or, TYPE(Or<1, 2>));

  // Address spaces
  ADDRESSSPACEMM(m_memory);
  ADDRESSSPACE(m_regMem);

  SUBCOMPONENT(ecallChecker, EcallChecker);

  // Ripes interface compliance
  const ProcessorStructure &structure() const override { return m_structure; }
  unsigned int getPcForStage(StageIndex idx) const override {
    // clang-format off
        switch (idx.index()) {
            case IF: return pc_reg->out.uValue();
            case ID: return ifid_reg->pc_out.uValue();
            case EX: return idex_reg->pc_out.uValue();
            case MEM: return exmem_reg->pc_out.uValue();
            case WB: return memwb_reg->pc_out.uValue();
            default: assert(false && "Processor does not contain stage");
        }
        Q_UNREACHABLE();
    // clang-format on
  }
  AInt nextFetchedAddress() const override {
    return br_nextpc->curr_next.uValue();
  }
  QString stageName(StageIndex idx) const override {
    // clang-format off
        switch (idx.index()) {
            case IF: return "IF";
            case ID: return "ID";
            case EX: return "EX";
            case MEM: return "MEM";
            case WB: return "WB";
            default: assert(false && "Processor does not contain stage");
        }
        Q_UNREACHABLE();
    // clang-format on
  }
  StageInfo stageInfo(StageIndex stage) const override {
    bool stageValid = true;
    // Has the pipeline stage been filled?
    stageValid &= stage.index() <= m_cycleCount;

    // clang-format off
        // Has the stage been cleared?
        switch(stage.index()){
        case ID: stageValid &= ifid_reg->valid_out.uValue(); break;
        case EX: stageValid &= idex_reg->valid_out.uValue(); break;
        case MEM: stageValid &= exmem_reg->valid_out.uValue(); break;
        case WB: stageValid &= memwb_reg->valid_out.uValue(); break;
        default: case IF: break;
        }

        // Is the stage carrying a valid (executable) PC?
        switch(stage.index()){
        case ID: stageValid &= isExecutableAddress(ifid_reg->pc_out.uValue()); break;
        case EX: stageValid &= isExecutableAddress(idex_reg->pc_out.uValue()); break;
        case MEM: stageValid &= isExecutableAddress(exmem_reg->pc_out.uValue()); break;
        case WB: stageValid &= isExecutableAddress(memwb_reg->pc_out.uValue()); break;
        default: case IF: stageValid &= isExecutableAddress(pc_reg->out.uValue()); break;
        }

        // Are we currently clearing the pipeline due to a syscall exit? if such, all stages before the EX stage are invalid
        if(stage.index() < EX){
            stageValid &= !ecallChecker->isSysCallExiting();
        }
    // clang-format on

    // Gather stage state info
    StageInfo::State state = StageInfo ::State::None;
    switch (stage.index()) {
    case IF:
      break;
    case ID:
      if (m_cycleCount > ID && ifid_reg->valid_out.uValue() == 0) {
        state = StageInfo::State::Flushed;
      }
      break;
    case EX: {
      if (idex_reg->stalled_out.uValue() == 1) {
        state = StageInfo::State::Stalled;
      } else if (m_cycleCount > EX && idex_reg->valid_out.uValue() == 0) {
        state = StageInfo::State::Flushed;
      }
      break;
    }
    case MEM: {
      if (exmem_reg->stalled_out.uValue() == 1) {
        state = StageInfo::State::Stalled;
      } else if (m_cycleCount > MEM && exmem_reg->valid_out.uValue() == 0) {
        state = StageInfo::State::Flushed;
      }
      break;
    }
    case WB: {
      if (memwb_reg->stalled_out.uValue() == 1) {
        state = StageInfo::State::Stalled;
      } else if (m_cycleCount > WB && memwb_reg->valid_out.uValue() == 0) {
        state = StageInfo::State::Flushed;
      }
      break;
    }
    }

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
  VInt getRegister(RegisterFileType, unsigned i) const override {
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
    return {{0, IF}};
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
    for (int stage = IF; stage < STAGECOUNT; stage++) {
      allStagesInvalid &= !stageInfo({0, stage}).stage_valid;
      if (!allStagesInvalid)
        break;
    }
    return allStagesInvalid;
  }

  void setRegister(RegisterFileType, unsigned i, VInt v) override {
    setSynchronousValue(registerFile->_wr_mem, i, v);
  }

  void clockProcessor() override {
    // An instruction has been retired if the instruction in the WB stage is
    // valid and the PC is within the executable range of the program
    if (memwb_reg->valid_out.uValue() != 0 &&
        isExecutableAddress(memwb_reg->pc_out.uValue())) {
      m_instructionsRetired++;
    }

    if (brunit->prev_is_b.uValue()) {
      PredictorHandler::getPredictor()->num_conditional++;
      if (brunit->prev_pre_miss.uValue()) {
        PredictorHandler::getPredictor()->num_conditional_miss++;
      }
    }

    PredictorHandler::clock();

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

    if (memwb_reg->valid_out.uValue() != 0 &&
        isExecutableAddress(memwb_reg->pc_out.uValue())) {
      m_instructionsRetired--;
    }

    if (brunit->prev_is_b.uValue()) {
      PredictorHandler::getPredictor()->num_conditional--;
      if (brunit->prev_pre_miss.uValue()) {
        PredictorHandler::getPredictor()->num_conditional_miss--;
      }
    }

    PredictorHandler::reverse();
  }

  void reset() override {
    ecallChecker->setSysCallExiting(false);
    Design::reset();
    m_syscallExitCycle = -1;
    PredictorHandler::getPredictor()->resetPredictorCounters();
  }

  static ProcessorISAInfo supportsISA() {
    return ProcessorISAInfo{
        std::make_shared<ISAInfo<XLenToRVISA<XLEN>()>>(QStringList()),
        {"M", "C"},
        {"M"}};
  }
  const ISAInfoBase *implementsISA() const override {
    return m_enabledISA.get();
  }

  const std::set<RegisterFileType> registerFiles() const override {
    std::set<RegisterFileType> rfs;
    rfs.insert(RegisterFileType::GPR);

    if (implementsISA()->extensionEnabled("F")) {
      rfs.insert(RegisterFileType::FPR);
    }
    return rfs;
  }

  bool supportsBranchPrediction() const override { return true; }

  // Branch Prediction

  bool currentInstructionIsBranch() override {
    unsigned opcode = uncompress->exp_instr.uValue() & 0x7F;
    switch (opcode) {
    case RVISA::Opcode::JAL:
    case RVISA::Opcode::JALR:
    case RVISA::Opcode::BRANCH:
      return true;
    default:
      return false;
    }
  }

  bool currentInstructionIsConditional() override {
    unsigned opcode = uncompress->exp_instr.uValue() & 0x7F;
    return opcode == RVISA::Opcode::BRANCH;
  }

  VInt currentInstructionImmediate() override {
    unsigned opcode = uncompress->exp_instr.uValue() & 0x7F;
    switch (opcode) {
    case RVISA::Opcode::JAL: {
      const auto fields = RVInstrParser::getParser()->decodeJ32Instr(
          uncompress->exp_instr.uValue());
      return VT_U(signextend<21>(fields[0] << 20 | fields[1] << 1 |
                                 fields[2] << 11 | fields[3] << 12));
    }
    case RVISA::Opcode::JALR: {
      return VT_U(signextend<12>((uncompress->exp_instr.uValue() >> 20)));
    }
    case RVISA::Opcode::BRANCH: {
      const auto fields = RVInstrParser::getParser()->decodeB32Instr(
          uncompress->exp_instr.uValue());
      return VT_U(signextend<13>((fields[0] << 12) | (fields[1] << 5) |
                                 (fields[5] << 1) | (fields[6] << 11)));
    }
    default:
      return 0;
    }
  }

  AInt currentInstructionTarget() override {
    if (!currentInstructionIsBranch()) {
      return 0;
    }
    return pc_reg->out.uValue() + currentInstructionImmediate();
  }

  bool currentGetPrediction() override {
    return brunit->curr_pre_take.uValue();
  }

private:
  /**
   * @brief m_syscallExitCycle
   * The variable will contain the cycle of which an exit system call was
   * executed. From this, we may determine when we roll back an exit system call
   * during rewinding.
   */
  long long m_syscallExitCycle = -1;
  std::shared_ptr<ISAInfoBase> m_enabledISA;
  ProcessorStructure m_structure = {{0, 5}};
};

} // namespace core
} // namespace vsrtl
