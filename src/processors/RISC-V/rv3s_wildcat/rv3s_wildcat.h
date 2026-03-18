#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_constant.h"
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
#include "processors/RISC-V/rv_uncompress.h"

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
    uncompress->setISA(m_enabledISA);

    // IF

    // PC register, increment mux, jump mux, adder

    // Instruction memory

    // IF/ID

    // Register file

    // Instruction register

    // ID

    // Decode

    // Immediate

    // Control

    // Operand mux

    // Memory address adder, data source mux

    // ID/EX

    // Data memory

    // Operand register

    // EX

    // Operand source mux

    // ALU

    // Output selector

  }

  // Subcomponents
  SUBCOMPONENT(registerFile, TYPE(RegisterFile<XLEN, true>));
  SUBCOMPONENT(alu, TYPE(ALU<XLEN>));
  SUBCOMPONENT(control, Control);
  SUBCOMPONENT(immediate, TYPE(Immediate<XLEN>));
  SUBCOMPONENT(decode, TYPE(Decode<XLEN>));
  SUBCOMPONENT(branch, TYPE(Branch<XLEN>));
  SUBCOMPONENT(pc_4, Adder<XLEN>);
  SUBCOMPONENT(mem_addr, Adder<XLEN>);
  SUBCOMPONENT(uncompress, TYPE(Uncompress<XLEN>));

  // Registers
  SUBCOMPONENT(pc_reg, Register<XLEN>);

  // Stage seperating registers
  SUBCOMPONENT(ifid_reg, TYPE(IFID<XLEN>));
  SUBCOMPONENT(idex_reg, TYPE(IDEX<XLEN>));

  // Multiplexers
  SUBCOMPONENT(reg_wr_src, TYPE(Multiplexer<2, XLEN>));
  SUBCOMPONENT(mem_wr_src, TYPE(Multiplexer<2, XLEN>));
  SUBCOMPONENT(mem_addr_fw, TYPE(Multiplexer<2, XLEN>));
  SUBCOMPONENT(ex_res_select, TYPE(Multiplexer<2, XLEN>));
  SUBCOMPONENT(pc_src, TYPE(EnumMultiplexer<PcSrc, XLEN>));
  SUBCOMPONENT(alu_op1_fw, TYPE(Multiplexer<2, XLEN>));
  SUBCOMPONENT(alu_op2_fw, TYPE(Multiplexer<2, XLEN>));
  SUBCOMPONENT(op2_src, TYPE(Multiplexer<2, XLEN>));
  SUBCOMPONENT(pc_inc, TYPE(EnumMultiplexer<PcInc, XLEN>));
  SUBCOMPONENT(pc_jump, TYPE(Multiplexer<2, XLEN>));


  // Memories
  SUBCOMPONENT(instr_mem, TYPE(ROM<XLEN, c_RVInstrWidth>));
  SUBCOMPONENT(data_mem, TYPE(RVMemory<XLEN, XLEN>));



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
    if (memwb_reg->valid_out.uValue() != 0 &&
        isExecutableAddress(memwb_reg->pc_out.uValue())) {
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
    if (memwb_reg->valid_out.uValue() != 0 &&
        isExecutableAddress(memwb_reg->pc_out.uValue())) {
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
