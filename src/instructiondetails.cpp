#include "instructiondetails.h"
#include "ui_instructiondetails.h"

#include "processorhandler.h"

namespace Ripes {

InstructionDetailsBase::InstructionDetailsBase(QObject *parent)
    : QObject(parent) {}

InstructionDetailsWindow::InstructionDetailsWindow(QWidget *parent)
      : QWidget(parent), m_ui(new Ui::InstructionDetails) {
  m_ui->setupUi(this);
  setWindowIcon(QIcon(":/icons/logo.svg"));
  setAssembler();
}

void InstructionDetailsWindow::setAssembler() {
  auto isa = ProcessorHandler::currentISA();
  auto assembler = ProcessorHandler::getAssembler();
  if (isa->bits() == 32) {
    m_instructionDetails = std::make_unique<InstructionDetails<uint32_t>>(
        std::dynamic_pointer_cast<Assembler::Assembler<uint32_t>>(assembler));
  } else if (isa->bits() == 64) {
    m_instructionDetails = std::make_unique<InstructionDetails<uint64_t>>(
        std::dynamic_pointer_cast<Assembler::Assembler<uint64_t>>(assembler));
  } else {
    // TODO(raccog): Should I make this an assertion?
    std::cerr << "Instruction details window not setup for " << isa->bits()
              << " bit register ISAs.\n";
  }
}

void InstructionDetailsWindow::setInstruction(AInt addr) {
  m_instructionDetails->setInstruction(m_ui, addr);
  show();
  activateWindow();
}

InstructionDetailsWindow::~InstructionDetailsWindow() {
  delete m_ui;
}

}
