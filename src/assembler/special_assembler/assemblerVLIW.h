#pragma once

#include "assemblerMultiIssue.h"
#include "isa/rv_i_ext.h"

namespace Ripes {
namespace Assembler {

class AssemblerVLIW : public AssemblerMultiIssueBase {
public:
  explicit AssemblerVLIW(std::shared_ptr<ISAInfoBase> isa)
  : AssemblerMultiIssueBase(isa) {
    // pre assemble nop instruction for nop way detection
    TokenizedSrcLine tsl(0);
    tsl.tokens = LineTokens() << Token("addi")
                              << Token("x0")
                              << Token("x0")
                              << Token("0");

    AssembleRes res = RVISA::ExtI::TypeI::Addi().assemble(tsl);
    
    if (res.isError()) {
      m_instr_nop = 0;
      return;
    }

    m_instr_nop = res.value().instruction;
  }

protected:
  virtual bool isValidWayInstruction(const AInt instructionIndex,
                                     const TokenizedSrcLine &line,
                                     const InstrRes &assembled,
                                     const std::shared_ptr<InstructionBase> instr,
                                     QString &error) const override {
    Q_UNUSED(line);

    // Nop's are always valid
    if (isNopInstruction(assembled)) {
      return true;
    }

    QString wayName = "unspecified";
    switch (instructionIndex % 2) {
      case 0: // Exec Way
        if (!isDataWayInstruction(instr)) {
          return true;
        }
        wayName = "Execution";
        break;
      
      case 1: // Data Way
        if (isDataWayInstruction(instr)) {
          return true;
        }
        wayName = "Data";
        break;
      
      default:
        break;
    }

    error = "Unrecognized Way Instruction, expected an <b>" + wayName + "</b>-Way instruction but got instruction '<b>" + instr->name() + "</b>'";
    return false;
  }
private:
  bool isNopInstruction(const InstrRes assembled) const {
    return assembled.instruction == m_instr_nop;
  }
  bool isDataWayInstruction(const std::shared_ptr<InstructionBase> instr) const {
    static const std::set<OpPartBase> dataWayInstructions = { 
      RVISA::OpPartOpcode<RVISA::OpcodeID::LOAD>(),
      RVISA::OpPartOpcode<RVISA::OpcodeID::STORE>()
    };
    
    return dataWayInstructions.count( instr->getOpPart(0 /*get opPartOpcode*/) ) != 0;
  }

  Instr_T m_instr_nop;
};

template <ISA isa>
using ISA_AssemblerVLIW = ISA_Assembler<isa, AssemblerVLIW>;

}
}