#pragma once

#include <QWidget>
#include "ui_instructiondetails.h"

#include "processorhandler.h"

#include "ripes_types.h"
#include "rvisainfo_common.h"

namespace Ripes {

namespace Ui {
class InstructionDetails;
}

class InstructionDetailsBase : public QObject {
  Q_OBJECT
public:
  InstructionDetailsBase(QObject *parent = nullptr);

  virtual void setInstruction(Ui::InstructionDetails *ui, AInt addr) = 0;
};

template <typename Reg_T>
class InstructionDetails : public InstructionDetailsBase {
public:
  InstructionDetails(std::shared_ptr<Assembler::Assembler<Reg_T>> assembler, QObject *parent = nullptr)
      : InstructionDetailsBase(parent), m_assembler(assembler) {}

  void setInstruction(Ui::InstructionDetails *ui, AInt addr) override {
    auto isa = ProcessorHandler::currentISA();
    VInt word = ProcessorHandler::getMemory().readMemConst(addr, isa->instrBits() / 8);
    auto program = ProcessorHandler::getProgram();
    auto instructionResult = m_assembler->getInstruction(word, program->symbols, addr);
    ui->address->setText("0x" + QString::number(addr, 16));
    ui->isa->setText(isa->name());
    ui->mnemonic->setText("Invalid instruction");
    ui->context->clear();
    ui->syntax->clear();
    ui->instrType->clear();
    if (auto *instruction = std::get<const Assembler::Instruction<Reg_T> *>(instructionResult)) {
      ui->mnemonic->setText(instruction->name());
      ui->context->setText(ProcessorHandler::disassembleInstr(addr));
      ui->syntax->setText(instruction->syntax());
//      ui->instrType->setText(RVISA::InstructionTypeNames.at(instruction->getInstructionType()));
    }
  }
private:
  std::shared_ptr<Assembler::Assembler<Reg_T>> m_assembler;
};

class InstructionDetailsWindow : public QWidget {
  Q_OBJECT
public:
  InstructionDetailsWindow(QWidget *parent = nullptr);

  ~InstructionDetailsWindow() override;

public slots:
  void setAssembler();
  void setInstruction(AInt addr);

private:
  Ui::InstructionDetails *m_ui;
  std::unique_ptr<InstructionDetailsBase> m_instructionDetails;
};


}
