#pragma once

#include "../assembler.h"

namespace Ripes {
namespace Assembler {

class AssemblerMultiIssueBase : public Assembler {
public:
  explicit AssemblerMultiIssueBase(std::shared_ptr<ISAInfoBase> isa)
    : Assembler(isa)
    {}

protected:
  virtual bool isValidWayInstruction(const AInt instructionIndex,
                                     const TokenizedSrcLine &line,
                                     const InstrRes &assembled,
                                     const std::shared_ptr<InstructionBase> instr,
                                     QString &error) const = 0;

  virtual void signalNewInstructionAssemblePass() const override {
    instructionCount = 0;
  }

  virtual AssembleRes
  assembleInstruction(const TokenizedSrcLine &line,
                      std::shared_ptr<InstructionBase> &assembledWith) const override {
    AssembleRes res = Assembler::assembleInstruction( line, assembledWith );
    
    if (res.isError()) {
      return res;
    }
    
    QString error;
    if( !isValidWayInstruction( instructionCount++, line, res.value(), assembledWith, error ) ) {
      return {Error(line, error)};
    }
    
    return res;
  }

private:
  mutable AInt instructionCount;
};

}
}