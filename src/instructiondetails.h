#pragma once

#include <QWidget>

namespace Ripes {

namespace Ui {
class InstructionDetails;
}

class InstructionDetails : public QWidget {
  Q_OBJECT
public:
  InstructionDetails(QWidget *parent = nullptr);
  ~InstructionDetails() override;

private:
  Ui::InstructionDetails *m_ui;
};

}
