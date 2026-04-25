#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "../../ripesvsrtlprocessor.h"

// Functional units
#include "processors/RISC-V/riscv.h"
#include "processors/RISC-V/rv_alu.h"
#include "processors/RISC-V/rv_branch.h"
#include "processors/RISC-V/rv_control.h"
#include "processors/RISC-V/rv_decode.h"
#include "processors/RISC-V/rv_ecallchecker.h"
#include "processors/RISC-V/rv_immediate.h"
#include "processors/RISC-V/rv_memory.h"
#include "processors/RISC-V/rv_registerfile.h"

// Stage separating registers
#include "rv3s_wildcat_ifid.h"
#include "rv3s_wildcat_idex.h"

// Forwarding & Hazard detection unit

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RV3S_WILDCAT : public RipesVSRTLProcessor {
  static_assert(std::is_same_v<uint32_t, XLEN_T> ||
                    std::is_same_v<uint64_t, XLEN_T>,
                "Only supports 32- and 64-bit variants");
  static constexpr unsigned XLEN = sizeof(XLEN_T) * CHAR_BIT;

public:
  enum Stage { IF = 0, ID = 1, EX = 2, STAGECOUNT };
  RV3S_WILDCAT(const QStringList &extensions)
      : RipesVSRTLProcessor("3-Stage RISC-V Wildcat Processor") {
    m_enabledISA = ISAInfoRegistry::getISA<XLenToRVISA<XLEN>()>(extensions);
    decode->setISA(m_enabledISA);

    // Component linking, grouped by stages and purpose.

    // IF
    pc_src->out >> pc_reg->in;
    4 >> pc_4->op1;
    pc_reg->out >> pc_4->op2;
    pc_reg->out >> instr_mem->addr;
    pc_4->out >> pc_src->get(PC4);

    // IF/ID register inputs
    pc_reg->out >> ifid_reg->pc_in;
    pc_4->out >> ifid_reg->pc_4_in;
    instr_mem->data_out >> ifid_reg->instr_in;

    // ID: Instruction inputs and decode outputs
    ifid_reg->instr_out >> decode->instr_in;
    ifid_reg->instr_out >> immediate->instr_in;
    decode->r1_reg_idx >> registerFile->r1_addr;
    decode->r2_reg_idx >> registerFile->r2_addr;
    decode->opcode >> immediate->opcode;
    decode->opcode >> control->opcode;

    // ID: Register and immediate values
    registerFile->r1_out >> alu_op1_src->get(AluSrc1::REG1);
    registerFile->r1_out >> branch->op1;
    registerFile->r1_out >> mem_addr->op1;

    registerFile->r2_out >> alu_op2_src->get(AluSrc2::REG2);
    registerFile->r2_out >> branch->op2;

    immediate->imm >> pc_target_addr->op2;
    immediate->imm >> alu_op2_src->get(AluSrc2::IMM);
    immediate->imm >> mem_addr->op2;

    // ID: Control unit signals consumed in ID.
    control->comp_ctrl >> branch->comp_op;
    control->alu_op1_ctrl >> alu_op1_src->select;
    control->do_branch >> *br_and->in[0];
    control->do_jump >> *controlflow_or->in[0];
    control->alu_op2_ctrl >> alu_op2_src->select;

    // ID: Branch and jump component outputs
    branch->res >> *br_and->in[1];
    alu_op1_src->out >> pc_target_addr->op1;
    br_and->out >> *controlflow_or->in[1];
    controlflow_or->out >> pc_src->select;
    pc_target_addr->out >> pc_src->get(PcSrc::ALU);

    // ID/EX register: Data inputs
    ifid_reg->pc_out >> idex_reg->pc_in;
    ifid_reg->pc_4_out >> idex_reg->pc_4_in;
    alu_op1_src->out >> idex_reg->op1_in;
    alu_op2_src->out >> idex_reg->op2_in;
    mem_addr->out >> idex_reg->mem_addr_in;
    registerFile->r2_out >> idex_reg->mem_wr_data_in;

    // ID/EX register: Signal inputs
    decode->wr_reg_idx >> idex_reg->wr_reg_idx_in;
    control->reg_do_write_ctrl >> idex_reg->reg_do_write_in;
    control->reg_wr_src_ctrl >> idex_reg->reg_wr_src_ctrl_in;
    control->alu_ctrl >> idex_reg->alu_ctrl_in;
    control->mem_do_write_ctrl >> idex_reg->mem_do_write_in;
    control->mem_ctrl >> idex_reg->mem_op_in;

    // EX: ALU inputs
    idex_reg->op1_out >> alu->op1;
    idex_reg->op2_out >> alu->op2;
    idex_reg->alu_ctrl_out >> alu->ctrl;

    // EX: Data memory inputs
    idex_reg->mem_addr_out >> data_mem->addr;
    idex_reg->mem_wr_data_out >> data_mem->data_in;
    idex_reg->mem_do_write_out >> data_mem->wr_en;
    idex_reg->mem_op_out >> data_mem->op;

    // EX: Write back
    idex_reg->pc_4_out >> ex_res_mux->get(RegWrSrc::PC4);
    alu->res >> ex_res_mux->get(RegWrSrc::ALURES);
    data_mem->data_out >> ex_res_mux->get(RegWrSrc::MEMREAD);
    idex_reg->reg_wr_src_ctrl_out >> ex_res_mux->select;
    ex_res_mux->out >> registerFile->data_in;
    idex_reg->reg_do_write_out >> registerFile->wr_en;

    // Memory assignments
    instr_mem->setMemory(m_memory);
    registerFile->setMemory(m_regMem);
    data_mem->mem->setMemory(m_memory);
  }

  // IF Components
  SUBCOMPONENT(pc_reg, Register<XLEN>);
  SUBCOMPONENT(instr_mem, TYPE(ROM<XLEN, c_RVInstrWidth>));
  ADDRESSSPACEMM(m_memory);
  SUBCOMPONENT(pc_4, Adder<XLEN>);
  SUBCOMPONENT(pc_src, TYPE(EnumMultiplexer<PcSrc, XLEN>));

  // IF/ID Register
  SUBCOMPONENT(ifid_reg, TYPE(Wildcat_IFID<XLEN>));

  // ID Components
  SUBCOMPONENT(decode, TYPE(Decode<XLEN>));
  SUBCOMPONENT(immediate, TYPE(Immediate<XLEN>));
  SUBCOMPONENT(registerFile, TYPE(RegisterFile<XLEN, true>));
  ADDRESSSPACE(m_regMem);
  SUBCOMPONENT(control, Control);
  SUBCOMPONENT(branch, TYPE(Branch<XLEN>));
  SUBCOMPONENT(alu_op1_src, TYPE(EnumMultiplexer<AluSrc1, XLEN>));
  SUBCOMPONENT(pc_target_addr, TYPE(Adder<XLEN>));
  SUBCOMPONENT(br_and, TYPE(And<1, 2>));
  SUBCOMPONENT(controlflow_or, TYPE(Or<1, 2>));
  SUBCOMPONENT(alu_op2_src, TYPE(EnumMultiplexer<AluSrc2, XLEN>));
  SUBCOMPONENT(mem_addr, Adder<XLEN>);


  // ID/EX Register
  SUBCOMPONENT(idex_reg, TYPE(Wildcat_IDEX<XLEN>));

  // EX Components
  SUBCOMPONENT(alu, TYPE(ALU<XLEN>));
  SUBCOMPONENT(data_mem, TYPE(RVMemory<XLEN, XLEN>));
  SUBCOMPONENT(ex_res_mux, TYPE(EnumMultiplexer<RegWrSrc, XLEN>));

  SUBCOMPONENT(ecallChecker, EcallChecker); // Interface compliance

  // Ripes interface compliance
  const ProcessorStructure &structure() const override { return m_structure; }
  unsigned int getPcForStage(StageIndex idx) const override {
    // clang-format off
        switch (idx.index()) {
            case IF: return pc_reg->out.uValue();
            case ID: return ifid_reg->pc_out.uValue();
            case EX: return idex_reg->pc_out.uValue();
            default: assert(false && "Processor does not contain stage");
        }
        Q_UNREACHABLE();
    // clang-format on
  }
  AInt nextFetchedAddress() const override { return pc_src->out.uValue(); }
  QString stageName(StageIndex idx) const override {
    // clang-format off
        switch (idx.index()) {
            case IF: return "IF";
            case ID: return "ID";
            case EX: return "EX";
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
        default: case IF: break;
        }

        // Is the stage carrying a valid (executable) PC?
        switch(stage.index()){
        case ID: stageValid &= isExecutableAddress(ifid_reg->pc_out.uValue()); break;
        case EX: stageValid &= isExecutableAddress(idex_reg->pc_out.uValue()); break;
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

  void setRegister(const std::string_view &, unsigned i, VInt v) override {
    setSynchronousValue(registerFile->_wr_mem, i, v);
  }

  void clockProcessor() override {
    // An instruction has been retired if the instruction in the WB stage is
    // valid and the PC is within the executable range of the program
    if (idex_reg->valid_out.uValue() != 0 &&
        isExecutableAddress(idex_reg->pc_out.uValue())) {
      m_instructionsRetired++;
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
    if (idex_reg->valid_out.uValue() != 0 &&
        isExecutableAddress(idex_reg->pc_out.uValue())) {
      m_instructionsRetired--;
    }
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
   * executed. From this, we may determine when we roll back an exit system call
   * during rewinding.
   */
  long long m_syscallExitCycle = -1;
  std::shared_ptr<ISAInfoBase> m_enabledISA;
  ProcessorStructure m_structure = {{0, 3}};
};

} // namespace core
} // namespace vsrtl
