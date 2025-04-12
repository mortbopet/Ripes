#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "processors/ripesvsrtlprocessor.h"

#include "processors/RISC-V/riscv.h"


#include "rv5mc_jump_unit.h"

#include "rv5mc_control.h"
#include "rv5mc_alu.h"

#include "processors/RISC-V/rv_ecallchecker.h"
#include "processors/RISC-V/rv_immediate.h"
#include "rv5mc_memory.h"
#include "processors/RISC-V/rv_registerfile.h"
#include "rv5mc_decode_and_compressor_register.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RV5MC : public RipesVSRTLProcessor {
  static_assert(std::is_same<uint32_t, XLEN_T>::value ||
                    std::is_same<uint64_t, XLEN_T>::value,
                "Only supports 32- and 64-bit variants");
  static constexpr unsigned XLEN = sizeof(XLEN_T) * CHAR_BIT;

public:
  enum Stage { IF = 0, ID = 1, EX = 2, MEM = 3, WB = 4, STAGECOUNT };
  RV5MC(const QStringList &extensions)
      : RipesVSRTLProcessor("Multicycle RISC-V Processor") {
    m_enabledISA = ISAInfoRegistry::getISA<XLenToRVISA<XLEN>()>(extensions);
    decode->setISA(m_enabledISA);

    // -----------------------------------------------------------------------
    // Program counter
    pc_src->out >> pc_reg->in;
    0 >> pc_reg->clear;
    controlflow_or->out >> pc_reg->enable;

    pc_reg->out >> pc_old_reg->in;
    0 >> pc_old_reg->clear;
    control->do_write_ir >> pc_old_reg->enable;

    2 >> pc_inc->get(PcInc::INC2);
    4 >> pc_inc->get(PcInc::INC4);
    decode->Pc_Inc >> pc_inc->select;

    // -----------------------------------------------------------------------
    // Instruction memory
    pc_reg->out >> instr_mem->addr;
    instr_mem->setMemory(m_memory);

    // -----------------------------------------------------------------------
    // Decode
    instr_mem->data_out >> decode->instr;
    control->do_write_ir >> decode->enable;

    // -----------------------------------------------------------------------
    // Control signals
    decode->opcode >> control->opcode;

    // -----------------------------------------------------------------------
    // Immediate
    decode->opcode >> immediate->opcode;
    decode->exp_instr >> immediate->instr;

    // -----------------------------------------------------------------------
    // Registers
    decode->wr_reg_idx >> registerFile->wr_addr;
    decode->r1_reg_idx >> registerFile->r1_addr;
    decode->r2_reg_idx >> registerFile->r2_addr;
    control->reg_do_write_ctrl >> registerFile->wr_en;
    reg_wr_src->out >> registerFile->data_in;

    data_mem->data_out >> men_out->in;
    men_out->out >> reg_wr_src->get(RegWrSrc::MEMREAD);
    ALU_out->out >> reg_wr_src->get(RegWrSrc::ALURES);
    pc_reg->out >> reg_wr_src->get(RegWrSrc::PC4); // TODO: FIXME
    control->reg_wr_src_ctrl >> reg_wr_src->select;
    control->do_read_mem >> data_mem->r;

    registerFile->setMemory(m_regMem);

    // -----------------------------------------------------------------------
    // Branch
//    control->comp_ctrl >> branch->comp_op;
//    registerFile->r1_out >> branch->op1;
//    registerFile->r2_out >> branch->op2;
//
//    branch->res >> *br_and->in[0];
      control->do_branch >> *br_and->in[1];
      br_and->out >> *controlflow_or->in[0];
      control->do_write_pc >> *controlflow_or->in[1];

    alu->sign >> jumcrv->sign;
    alu->zero >> jumcrv->zero;
    alu->carry >> jumcrv->carry;
    control->comp_ctrl >> jumcrv->comp_op;
    jumcrv->res >> *br_and->in[0];

    // -----------------------------------------------------------------------
    // ALU
    registerFile->r1_out >> a->in;
    a->out >> alu_op1_src->get(AluSrc1MC::REG1);
    pc_reg->out >> alu_op1_src->get(AluSrc1MC::PC);
    pc_old_reg->out >> alu_op1_src->get(AluSrc1MC::PCOLD);
    control->alu_op1_ctrl >> alu_op1_src->select;

    registerFile->r2_out >> b->in;
    b->out >> alu_op2_src->get(AluSrc2MC::REG2);
    immediate->imm >> alu_op2_src->get(AluSrc2MC::IMM);

    pc_inc->out >> alu_op2_src->get(AluSrc2MC::INPC);

    control->alu_op2_ctrl >> alu_op2_src->select;

    alu_op1_src->out >> alu->op1;
    alu_op2_src->out >> alu->op2;

    control->alu_ctrl >> alu->ctrl;

    alu->res >> ALU_out->in;

    ALU_out->out >> pc_src->get(PcSrc::ALU);
    alu->res >> pc_src->get(PcSrc::PC4);

    control->pc_scr_ctrl >> pc_src->select;

    // -----------------------------------------------------------------------
    // Data memory
    ALU_out->out >> data_mem->addr;
    control->do_write_mem >> data_mem->wr_en;
    b->out >> data_mem->data_in;
    control->mem_ctrl >> data_mem->op;
    data_mem->mem->setMemory(m_memory);

    // -----------------------------------------------------------------------
    // Ecall checker
    decode->opcode >> ecallChecker->opcode;
    ecallChecker->setSyscallCallback(&trapHandler);
    control->ecall_ctr >> ecallChecker->stallEcallHandling;
  }

  // Design subcomponents
  SUBCOMPONENT(registerFile, TYPE(RegisterFile<XLEN, false>));
  SUBCOMPONENT(alu, TYPE(RVMCALU<XLEN>));
  SUBCOMPONENT(control, RVMCControl);
  SUBCOMPONENT(immediate, TYPE(Immediate<XLEN>));
  SUBCOMPONENT(decode, TYPE(DecodeRVMC<XLEN>));

  SUBCOMPONENT(jumcrv, TYPE(JURVMC<XLEN>));

  // Registers
  SUBCOMPONENT(pc_old_reg, RegisterClEn<XLEN>);
  SUBCOMPONENT(pc_reg, RegisterClEn<XLEN>);
  SUBCOMPONENT(a,Register<XLEN>);
  SUBCOMPONENT(b,Register<XLEN>);
  SUBCOMPONENT(ALU_out,Register<XLEN>);
  SUBCOMPONENT(men_out,Register<XLEN>);

  // Multiplexers
  SUBCOMPONENT(reg_wr_src, TYPE(EnumMultiplexer<RegWrSrc, XLEN>));
  SUBCOMPONENT(pc_src, TYPE(EnumMultiplexer<PcSrc, XLEN>));
  SUBCOMPONENT(alu_op1_src, TYPE(EnumMultiplexer<AluSrc1MC, XLEN>));
  SUBCOMPONENT(alu_op2_src, TYPE(EnumMultiplexer<AluSrc2MC, XLEN>));
  SUBCOMPONENT(pc_inc, TYPE(EnumMultiplexer<PcInc, XLEN>));

  // Memories
  SUBCOMPONENT(instr_mem, TYPE(ROM<XLEN, c_RVInstrWidth>));
  SUBCOMPONENT(data_mem, TYPE(RVMCMemory<XLEN, XLEN>));

  // Gates
  SUBCOMPONENT(br_and, TYPE(And<1, 2>));
  SUBCOMPONENT(controlflow_or, TYPE(Or<1, 2>));

  // Address spaces
  ADDRESSSPACEMM(m_memory);
  ADDRESSSPACE(m_regMem);

  SUBCOMPONENT(ecallChecker, EcallChecker);

  // Ripes interface compliance
  const ProcessorStructure &structure() const override { return m_structure; }
  unsigned int getPcForStage(StageIndex stage) const override {
    switch (stage.index()) {
    case EX: case MEM:
      return pc_old_reg->out.uValue();
    default:
      return pc_reg->out.uValue();
    }
  }
  AInt nextFetchedAddress() const override {
    return pc_src->out.uValue();
  }
  QString stageName(StageIndex stage) const override {
    // clang-format off
        switch (stage.index()) {
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

        //State valid
        switch (stage.index()) {
        case IF:
          stageValid &= (control->stateRegCtr->out.eValue<FSMState>() == FSMState::IF);
          break;
        case ID:
          stageValid &= (control->stateRegCtr->out.eValue<FSMState>() == FSMState::ID);
          break;
        case EX:
          stageValid &= (control->stateRegCtr->out.eValue<FSMState>() >= FSMState::EX && control->stateRegCtr->out.eValue<FSMState>() < FSMState::MEM);
          break;
        case MEM:
          stageValid &= (control->stateRegCtr->out.eValue<FSMState>() >= FSMState::MEM && control->stateRegCtr->out.eValue<FSMState>() < FSMState::WB);
          break;
        case WB:
          stageValid &= (control->stateRegCtr->out.eValue<FSMState>() >= FSMState::WB);
          break;
        default: break;
        }

        // Gather stage state info
        StageInfo::State state = StageInfo ::State::None;

    return StageInfo({getPcForStage(stage),
                      stageValid,
                      state});
  }
  void setProgramCounter(AInt address) override {
    pc_reg->forceValue(0,address);
    // FIXME: set also current state to IF?
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
    if (fr == FinalizeReason::exitSyscall) {
      // Allow one additional clock cycle to clear the current instruction
      m_finishInNextCycle = true;
    }
  }
  bool finished() const override {
    return m_finished || ((!isExecutableAddress(pc_reg->out.uValue()) && !isExecutableAddress(pc_reg->in.uValue())) && control->do_finish_this_cycle());

  }
  const std::vector<StageIndex> breakpointTriggeringStages() const override {
    return {{0, 0}};
  }

  MemoryAccess dataMemAccess() const override {
    return memToAccessInfo(data_mem);
  }
  MemoryAccess instrMemAccess() const override {
    auto instrAccess = memToAccessInfo(instr_mem);
    instrAccess.type = MemoryAccess::Read;
    return instrAccess;
  }

  void setRegister(const std::string_view &, unsigned i, VInt v) override {
    setSynchronousValue(registerFile->_wr_mem, i, v);
  }

  void clockProcessor() override {
    if (control->do_finish_this_cycle())
      m_instructionsRetired++;


    // m_finishInNextCycle may be set during Design::clock(). Store the value
    // before clocking the processor, and emit finished if this was the final
    // clock cycle.
    const bool finishInThisCycle = m_finishInNextCycle;
    Design::clock();
    if (finishInThisCycle) {
      m_finished = true;
    }
  }

  void reverse() override {
    if(control->stateInPort.eValue<FSMState>()==FSMState::IF) m_instructionsRetired--;

    Design::reverse();
    // Ensure that reverses performed when we expected to finish in the
    // following cycle, clears this expectation.
    m_finishInNextCycle = false;
    m_finished = false;
  }

  void reset() override {
    Design::reset();
    m_finishInNextCycle = false;
    m_finished = false;
  }

  static ProcessorISAInfo supportsISA() {
    return ProcessorISAInfo{
        std::make_shared<ISAInfo<XLenToRVISA<XLEN>()>>(QStringList()),
        {"M", "C"},
        {"M"}};
  }
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
  bool m_finishInNextCycle = false;
  bool m_finished = false;
  std::shared_ptr<ISAInfoBase> m_enabledISA;
  ProcessorStructure m_structure = {{0, 5}};
};

} // namespace core
} // namespace vsrtl
