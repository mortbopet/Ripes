#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "processors/ripesvsrtlprocessor.h"

#include "processors/RISC-V/riscv.h"
#include "processors/RISC-V/rv_branch.h"


#include "rv5mc_jump_unit.h"

#include "rv5mc_control.h"
#include "rv5mc_pc.h"
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
      : RipesVSRTLProcessor("Single Cycle RISC-V Processor") {
    m_enabledISA = ISAInfoRegistry::getISA<XLenToRVISA<XLEN>()>(extensions);
    decode->setISA(m_enabledISA);

    // -----------------------------------------------------------------------
    // Program counter
    //pc_reg->pc_out >> pc_4->op1;
    //pc_inc->out >> pc_4->op2;

    pc_src->out >> pc_reg->pc_in;
    pc_src->out >> pc_old_in_scr->get(PCOldInscr::PCin);
    pc_reg->pc_out >> pc_old_in_scr->get(PCOldInscr::PCout);

    control->pc_old_w >> pc_old_reg->enable;

    br_and->out >> *controlflow_old_pc_o->in[0];
    control->pc_old_in_scr_ctrl >> *controlflow_old_pc_o->in[1];
    controlflow_old_pc_o->out >> pc_old_in_scr->select;


    pc_old_in_scr->out >> pc_old_reg->pc_in;


    pc_reg->pc_out >> pc_out_scr->get(PCOutscr::PC);
    pc_old_reg->pc_out >> pc_out_scr->get(PCOutscr::PCOld);
    control->pc_out_scr_crtl >> pc_out_scr->select;



    2 >> pc_inc->get(PcInc::INC2);
    4 >> pc_inc->get(PcInc::INC4);
    decode->Pc_Inc >> pc_inc->select;


    // Note: pc_src works uses the PcSrc enum, but is selected by the boolean
    // signal from the controlflow OR gate. PcSrc enum values must adhere to the
    // boolean 0/1 values.




    // -----------------------------------------------------------------------
    // Instruction memory
    pc_reg->pc_out >> instr_mem->addr;
    instr_mem->setMemory(m_memory);

    // -----------------------------------------------------------------------
    // Decode
    instr_mem->data_out >> decode->instr;
    control->RIWrite >> decode->enable;

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
    pc_out_scr->out >> reg_wr_src->get(RegWrSrc::PC4);
    control->reg_wr_src_ctrl >> reg_wr_src->select;
    control->read_men_ctrl >> data_mem->r;

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

      controlflow_or->out >> pc_reg->enable;

    alu->sign >> jumcrv->sign;
    alu->zero >> jumcrv->zero;
    alu->carry >> jumcrv->carry;
    control->comp_ctrl >> jumcrv->comp_op;
    jumcrv->res >> *br_and->in[0];

    // -----------------------------------------------------------------------
    // ALU
    registerFile->r1_out >> a->in;
    a->out >> alu_op1_src->get(AluSrc1::REG1);
    pc_out_scr->out >> alu_op1_src->get(AluSrc1::PC);
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
    control->mem_do_write_ctrl >> data_mem->wr_en;
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
  SUBCOMPONENT(pc_old_reg, RVMCPC<XLEN>);
  SUBCOMPONENT(pc_reg, RVMCPC<XLEN>);
  SUBCOMPONENT(a,Register<XLEN>);
  SUBCOMPONENT(b,Register<XLEN>);
  SUBCOMPONENT(ALU_out,Register<XLEN>);
  SUBCOMPONENT(men_out,Register<XLEN>);

  // Multiplexers
  SUBCOMPONENT(reg_wr_src, TYPE(EnumMultiplexer<RegWrSrc, XLEN>));
  SUBCOMPONENT(pc_src, TYPE(EnumMultiplexer<PcSrc, XLEN>));
  SUBCOMPONENT(alu_op1_src, TYPE(EnumMultiplexer<AluSrc1, XLEN>));
  SUBCOMPONENT(alu_op2_src, TYPE(EnumMultiplexer<AluSrc2MC, XLEN>));
  SUBCOMPONENT(pc_inc, TYPE(EnumMultiplexer<PcInc, XLEN>));

  SUBCOMPONENT(pc_out_scr, TYPE(EnumMultiplexer<PCOutscr, XLEN>));
  SUBCOMPONENT(pc_old_in_scr, TYPE(EnumMultiplexer<PCOldInscr, XLEN>));


  // Memories
  SUBCOMPONENT(instr_mem, TYPE(ROM<XLEN, c_RVInstrWidth>));
  SUBCOMPONENT(data_mem, TYPE(RVMCMemory<XLEN, XLEN>));

  // Gates
  SUBCOMPONENT(br_and, TYPE(And<1, 2>));
  SUBCOMPONENT(controlflow_or, TYPE(Or<1, 2>));
  SUBCOMPONENT(controlflow_old_pc_o, TYPE(Or<1, 2>));

  // Address spaces
  ADDRESSSPACEMM(m_memory);
  ADDRESSSPACE(m_regMem);

  SUBCOMPONENT(ecallChecker, EcallChecker);

  // Ripes interface compliance
  const ProcessorStructure &structure() const override { return m_structure; }
  unsigned int getPcForStage(StageIndex stage) const override {
    switch (stage.index()) {
    case EX: case MEM:
      return pc_old_reg->pc_out.uValue();
    default:
      return pc_out_scr->out.uValue();
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
    pc_reg->pc_reg->forceValue(0,address);
    propagateDesign();
  }
  void setPCInitialValue(AInt address) override {
    pc_reg->pc_reg->setInitValue(address);
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
    return m_finished || ((!isExecutableAddress(pc_reg->pc_out.uValue()) && !isExecutableAddress(pc_reg->pc_in.uValue())) && control->do_finish_this_cycle());

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
    // Single cycle processor; 1 instruction retired per cycle!
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
    if(control->stateIntPort.eValue<FSMState>()==FSMState::IF) m_instructionsRetired--;

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
  const ISAInfoBase *implementsISA() const override {
    return m_enabledISA.get();
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
