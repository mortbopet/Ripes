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
#include "processors/RISC-V/rv_ecallchecker.h"
#include "processors/RISC-V/rv_immediate.h"
#include "processors/RISC-V/rv_memory.h"
#include "processors/RISC-V/rv_registerfile.h"

namespace vsrtl {
namespace core {
using namespace Ripes;
using namespace Ripes::RVISA;

template <typename XLEN_T>
class RVSS_FLOAT : public RipesVSRTLProcessor {
  static_assert(std::is_same<uint32_t, XLEN_T>::value ||
                    std::is_same<uint64_t, XLEN_T>::value,
                "Only supports 32- and 64-bit variants");
  static constexpr unsigned XLEN = sizeof(XLEN_T) * CHAR_BIT;

public:
  RVSS_FLOAT(const ExtensionSetInfo &extensions)
      : RipesVSRTLProcessor("Single Cycle RISC-V floating point Processor") {
    RV_ExtensionSet exts{dynamic_cast<const RV_ExtensionSet &>(extensions)};
    exts << Extension::F; // enforce F extension, since the processor is
                          // designed to work with it

    m_enabledISA = ISAInfoRegistry::getISA<XLenToRVISA<XLEN>()>(exts);
    decode->setISA(m_enabledISA);
    uncompress->setISA(m_enabledISA);

    // -----------------------------------------------------------------------
    // Program counter
    pc_reg->out >> pc_4->op1;
    pc_inc->out >> pc_4->op2;
    pc_src->out >> pc_reg->in;

    2 >> pc_inc->get(PcInc::INC2);
    4 >> pc_inc->get(PcInc::INC4);
    uncompress->Pc_Inc >> pc_inc->select;

    // Note: pc_src works uses the PcSrc enum, but is selected by the boolean
    // signal from the controlflow OR gate. PcSrc enum values must adhere to the
    // boolean 0/1 values.
    controlflow_or->out >> pc_src->select;

    // -----------------------------------------------------------------------
    // Instruction memory
    pc_reg->out >> instr_mem->addr;
    instr_mem->setMemory(m_memory);

    // -----------------------------------------------------------------------
    // Decode
    instr_mem->data_out >> uncompress->instr;
    uncompress->exp_instr >> decode->instr;

    // -----------------------------------------------------------------------
    // Control signals
    decode->opcode >> control->opcode;
    decode->csr_reg_idx >> control->csr_reg_idx;

    // -----------------------------------------------------------------------
    // Immediate
    decode->opcode >> immediate->opcode;
    uncompress->exp_instr >> immediate->instr;

    // -----------------------------------------------------------------------
    // Integer Registers
    decode->wr_reg_idx >> registerFileI->wr_addr;
    decode->r1_reg_idx >> registerFileI->r1_addr;
    decode->r2_reg_idx >> registerFileI->r2_addr;

    reg_wr_src->out >> registerFileI->data_in;
    control->reg_do_write_ctrl >> registerFileI->wr_en;

    registerFileI->setMemory(m_IregMem);

    // -----------------------------------------------------------------------
    // Floating Point Registers
    decode->wr_reg_idx >> registerFileF->wr_addr;
    decode->r1_reg_idx >> registerFileF->r1_addr;
    decode->r2_reg_idx >> registerFileF->r2_addr;
    decode->r3_reg_idx >> registerFileF->r3_addr;

    reg_wr_src->out >> registerFileF->data_in;
    control->regF_do_write_ctrl >> registerFileF->wr_en;

    registerFileF->setMemory(m_FregMem);

    // -----------------------------------------------------------------------
    // Register MUXes
    registerFileI->r1_out >> reg1_src->get(ImmRegFileSrc::INTEGER);
    registerFileF->r1_out >> reg1_src->get(ImmRegFileSrc::FLOAT);
    immediate->imm >> reg1_src->get(ImmRegFileSrc::IMMEDIATE);
    control->reg1_do_read_src >> reg1_src->select;

    registerFileI->r2_out >> reg2_src->get(RegFileSrc::INTEGER);
    registerFileF->r2_out >> reg2_src->get(RegFileSrc::FLOAT);
    control->reg2_do_read_src >> reg2_src->select;

    data_mem->data_out >> reg_wr_src->get(RegWrSrc::MEMREAD);
    alu_fpu_src->out >> reg_wr_src->get(RegWrSrc::ALURES);
    pc_4->out >> reg_wr_src->get(RegWrSrc::PC4);
    control->reg_wr_src_ctrl >> reg_wr_src->select;

    // -----------------------------------------------------------------------
    // Branch
    control->comp_ctrl >> branch->comp_op;
    registerFileI->r1_out >> branch->op1;
    registerFileI->r2_out >> branch->op2;

    branch->res >> *br_and->in[0];
    control->do_branch >> *br_and->in[1];
    br_and->out >> *controlflow_or->in[0];
    control->do_jump >> *controlflow_or->in[1];
    pc_4->out >> pc_src->get(PcSrc::PC4);
    alu->res >> pc_src->get(PcSrc::ALU);

    // -----------------------------------------------------------------------
    // ALU
    registerFileI->r1_out >> alu_op1_src->get(AluSrc1::REG1);
    pc_reg->out >> alu_op1_src->get(AluSrc1::PC);
    control->alu_op1_ctrl >> alu_op1_src->select;

    registerFileI->r2_out >> alu_op2_src->get(AluSrc2::REG2);
    immediate->imm >> alu_op2_src->get(AluSrc2::IMM);
    control->alu_op2_ctrl >> alu_op2_src->select;

    alu_op1_src->out >> alu->op1;
    alu_op2_src->out >> alu->op2;

    control->alu_ctrl >> alu->ctrl;

    // -----------------------------------------------------------------------
    // FPU
    reg1_src->out >> fpu->op1;
    registerFileF->r2_out >> fpu->op2;
    registerFileF->r3_out >> fpu->op3;

    control->fpu_ctrl >> fpu->ctrl;

    decode->roundMode >> fpu->roundmode;

    alu->res >> alu_fpu_src->get(RegFileSrc::INTEGER);
    fpu->res >> alu_fpu_src->get(RegFileSrc::FLOAT);
    control->alu_fpu_src >> alu_fpu_src->select;

    // -----------------------------------------------------------------------
    // Data memory
    alu_fpu_src->out >> data_mem->addr;
    control->mem_do_write_ctrl >> data_mem->wr_en;
    reg2_src->out >> data_mem->data_in;
    control->mem_ctrl >> data_mem->op;
    data_mem->mem->setMemory(m_memory);

    // -----------------------------------------------------------------------
    // Ecall checker
    decode->opcode >> ecallChecker->opcode;
    ecallChecker->setSyscallCallback(&trapHandler);
    0 >> ecallChecker->stallEcallHandling;
  }

  // Design subcomponents
  SUBCOMPONENT(registerFileI,
               TYPE(RegisterFile<XLEN, XLEN, false, c_RVRegs, true>));
  SUBCOMPONENT(registerFileF,
               TYPE(RegisterFile3<XLEN, c_RVFBits, false, c_RVFRegs, false>));
  SUBCOMPONENT(alu, TYPE(ALU<XLEN>));
  SUBCOMPONENT(fpu, TYPE(FPU_Fcsr<XLEN>));
  SUBCOMPONENT(control, ControlF);
  SUBCOMPONENT(immediate, TYPE(Immediate<XLEN>));
  SUBCOMPONENT(decode, TYPE(DecodeF<XLEN>));
  SUBCOMPONENT(branch, TYPE(Branch<XLEN>));
  SUBCOMPONENT(pc_4, Adder<XLEN>);
  SUBCOMPONENT(uncompress, TYPE(Uncompress<XLEN>));

  // Registers
  SUBCOMPONENT(pc_reg, Register<XLEN>);

  // Multiplexers
  SUBCOMPONENT(alu_fpu_src, TYPE(EnumMultiplexer<RegFileSrc, XLEN>));
  SUBCOMPONENT(reg1_src, TYPE(EnumMultiplexer<ImmRegFileSrc, XLEN>));
  SUBCOMPONENT(reg2_src, TYPE(EnumMultiplexer<RegFileSrc, XLEN>));
  SUBCOMPONENT(reg_wr_src, TYPE(EnumMultiplexer<RegWrSrc, XLEN>));
  SUBCOMPONENT(pc_src, TYPE(EnumMultiplexer<PcSrc, XLEN>));
  SUBCOMPONENT(alu_op1_src, TYPE(EnumMultiplexer<AluSrc1, XLEN>));
  SUBCOMPONENT(alu_op2_src, TYPE(EnumMultiplexer<AluSrc2, XLEN>));
  SUBCOMPONENT(pc_inc, TYPE(EnumMultiplexer<PcInc, XLEN>));

  // Memories
  SUBCOMPONENT(instr_mem, TYPE(ROM<XLEN, c_RVInstrWidth>));
  SUBCOMPONENT(data_mem, TYPE(RVMemory<XLEN, XLEN>));

  // Gates
  SUBCOMPONENT(br_and, TYPE(And<1, 2>));
  SUBCOMPONENT(controlflow_or, TYPE(Or<1, 2>));

  // Address spaces
  ADDRESSSPACEMM(m_memory);
  ADDRESSSPACE(m_IregMem);
  ADDRESSSPACE(m_FregMem);

  SUBCOMPONENT(ecallChecker, EcallChecker);

  // Ripes interface compliance
  const ProcessorStructure &structure() const override { return m_structure; }
  unsigned int getPcForStage(StageIndex) const override {
    return pc_reg->out.uValue();
  }
  AInt nextFetchedAddress() const override { return pc_src->out.uValue(); }
  QString stageName(StageIndex) const override { return "•"; }
  StageInfo stageInfo(StageIndex) const override {
    return StageInfo({pc_reg->out.uValue(),
                      isExecutableAddress(pc_reg->out.uValue()),
                      StageInfo::State::None});
  }
  void setProgramCounter(AInt address) override {
    pc_reg->forceValue(0, address);
    propagateDesign();
  }
  void setPCInitialValue(AInt address) override {
    pc_reg->setInitValue(address);
  }
  AddressSpaceMM &getMemory() override { return *m_memory; }
  VInt getRegister(const std::string_view &regfile, unsigned i) const override {
    if (regfile == RVISA::GPR) {
      return registerFileI->getRegister(i);
    } else if (regfile == RVISA::FPR) {
      return registerFileF->getRegister(i);
    }

    Q_ASSERT(false && "Unknown register file");
    return 0;
  }
  void finalize(FinalizeReason fr) override {
    if (fr == FinalizeReason::exitSyscall) {
      // Allow one additional clock cycle to clear the current instruction
      m_finishInNextCycle = true;
    }
  }
  bool finished() const override {
    return m_finished || !stageInfo({0, 0}).stage_valid;
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

  void setRegister(const std::string_view &regfile, unsigned i,
                   VInt v) override {
    if (regfile == RVISA::GPR)
      return setSynchronousValue(registerFileI->_wr_mem, i, v);
    else if (regfile == RVISA::FPR)
      return setSynchronousValue(registerFileF->_wr_mem, i, v);

    Q_ASSERT(false && "Unknown register file for this processor");
  }

  void clockProcessor() override {
    // Single cycle processor; 1 instruction retired per cycle!
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
    m_instructionsRetired--;
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

  static const ProcessorISAInfo &supportsISA() {
    static ProcessorISAInfo procInfo{
        std::make_shared<ISAInfo<XLenToRVISA<XLEN>()>>(),
        std::make_shared<RV_ExtensionSet>(Extension::M, Extension::F,
                                          Extension::C),
        std::make_shared<RV_ExtensionSet>(Extension::M, Extension::F)};
    return procInfo;
  }
  std::shared_ptr<ISAInfoBase> implementsISA() const override {
    return m_enabledISA;
  }
  std::shared_ptr<const ISAInfoBase> fullISA() const override {
    return std::make_shared<ISAInfo<XLenToRVISA<XLEN>()>>(
        *(supportsISA().supportedExtensions));
  }

  const std::set<std::string_view> registerFiles() const override {
    std::set<std::string_view> rfs;
    rfs.insert(RVISA::GPR);
    rfs.insert(RVISA::FPR);
    return rfs;
  }

private:
  bool m_finishInNextCycle = false;
  bool m_finished = false;
  std::shared_ptr<ISAInfoBase> m_enabledISA;
  ProcessorStructure m_structure = {{0, 1}};
};

} // namespace core
} // namespace vsrtl
