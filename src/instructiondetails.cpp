#include "instructiondetails.h"
#include "ui_instructiondetails.h"

namespace Ripes {

InstructionDetails::InstructionDetails(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::InstructionDetails) {
  m_ui->setupUi(this);
}

InstructionDetails::~InstructionDetails() {
  delete m_ui;
}

}
