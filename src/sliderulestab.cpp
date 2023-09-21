#include "sliderulestab.h"
#include "ui_sliderulestab.h"

namespace Ripes {

QStringList HEADER_LABELS = { "Instr", "31", "30", "29", "28", "27", "26",
                             "25", "24", "23", "22", "21", "20", "19", "18",
                             "17", "16", "15", "14", "13", "12", "11", "10",
                             "9", "8", "7", "6", "5", "4", "3", "2", "1", "0"};

SliderulesTab::SliderulesTab(QToolBar *toolbar, QWidget *parent)
    : RipesTab(toolbar, parent), m_ui(new Ui::SliderulesTab) {
  m_ui->setupUi(this);

  m_ui->tableWidget->setColumnWidth(0, 100);
  m_ui->tableWidget->setHorizontalHeaderLabels(HEADER_LABELS);

  auto *cellTemplate = new QTableWidgetItem("");
  cellTemplate->setTextAlignment(Qt::AlignCenter);
  auto *cellTemplateBold = cellTemplate->clone();
  auto font = cellTemplateBold->font();
  font.setBold(true);
  cellTemplateBold->setFont(font);
  auto *redItem = cellTemplateBold->clone();
  redItem->setBackground(QBrush(QColor::fromRgb(224, 102, 102)));
  auto *greenItem = cellTemplateBold->clone();
  greenItem->setBackground(QBrush(QColor::fromRgb(147, 196, 125)));
  auto *grayItem = cellTemplate->clone();
  grayItem->setBackground(QBrush(QColor::fromRgb(239, 239, 239)));

  auto *lightRedItem = cellTemplateBold->clone();
  lightRedItem->setBackground(QBrush(QColor::fromRgb(234, 153, 153)));
  auto *brightRedItem = cellTemplateBold->clone();
  brightRedItem->setBackground(QBrush(QColor::fromRgb(244, 204, 204)));
  auto *lightGreenItem = cellTemplateBold->clone();
  lightGreenItem->setBackground(QBrush(QColor::fromRgb(182, 215, 168)));
  auto *brightGreenItem = cellTemplateBold->clone();
  brightGreenItem->setBackground(QBrush(QColor::fromRgb(217, 234, 211)));

  auto *zeroLightRed = lightRedItem->clone();
  zeroLightRed->setText("0");
  auto *dashLightRed = lightRedItem->clone();
  dashLightRed->setText("-");
  auto *zeroBrightRed = brightRedItem->clone();
  zeroBrightRed->setText("0");
  auto *oneBrightRed = brightRedItem->clone();
  oneBrightRed->setText("1");
  auto *oneWhite = cellTemplate->clone();
  oneWhite->setText("1");
  auto *m1Green = greenItem->clone();
  m1Green->setText("m1");
  auto *m1ExtendedGreen = greenItem->clone();
  m1ExtendedGreen->setText("- m1 -");
  auto *imm1Green = greenItem->clone();
  imm1Green->setText("imm1");
  auto *zeroLightGreen = lightGreenItem->clone();
  zeroLightGreen->setText("0");
  auto *zeroBrightGreen = brightGreenItem->clone();
  zeroBrightGreen->setText("0");
  auto *oneBrightGreen = brightGreenItem->clone();
  oneBrightGreen->setText("1");

  // add
  //  instruction
  auto *add = redItem->clone();
  add->setText("add");
  m_ui->tableWidget->setItem(0, 0, add);
  //  funct7
  for (int i = 0; i < 7; ++i) {
    QTableWidgetItem *item;
    if (i == 1) {
      item = zeroLightRed->clone();
    } else {
      item = dashLightRed->clone();
    }
    m_ui->tableWidget->setItem(0, i + 1, item);
  }
  //  rs2
  auto *rs2 = grayItem->clone();
  rs2->setText("rs2");
  m_ui->tableWidget->setItem(0, 8, rs2);
  m_ui->tableWidget->setSpan(0, 8, 1, 5);
  //  rs1
  auto *rs1 = grayItem->clone();
  rs1->setText("rs1");
  m_ui->tableWidget->setItem(0, 13, rs1);
  m_ui->tableWidget->setSpan(0, 13, 1, 5);
  //  funct3
  for (int i = 0; i < 3; ++i) {
    m_ui->tableWidget->setItem(0, i + 18, zeroLightRed->clone());
  }
  //  rd
  auto *rd= grayItem->clone();
  rd->setText("rd");
  m_ui->tableWidget->setItem(0, 21, rd);
  m_ui->tableWidget->setSpan(0, 21, 1, 5);
  //  opcode
  m_ui->tableWidget->setItem(0, 26, zeroBrightRed->clone());
  m_ui->tableWidget->setItem(0, 27, oneBrightRed->clone());
  m_ui->tableWidget->setItem(0, 28, oneBrightRed->clone());
  m_ui->tableWidget->setItem(0, 29, zeroBrightRed->clone());
  m_ui->tableWidget->setItem(0, 30, zeroBrightRed->clone());
  //  quadrant
  m_ui->tableWidget->setItem(0, 31, oneWhite->clone());
  m_ui->tableWidget->setItem(0, 32, oneWhite->clone());

  // addi
  //  instruction
  auto *addi = greenItem->clone();
  addi->setText("addi");
  m_ui->tableWidget->setItem(1, 0, addi);
  m_ui->tableWidget->setSpan(1, 0, 2, 1);
  //  imm
  m_ui->tableWidget->setItem(1, 1, m1Green->clone());
  m_ui->tableWidget->setItem(1, 2, imm1Green->clone());
  m_ui->tableWidget->setSpan(1, 2, 1, 11);
  //  rs
  auto *rs = grayItem->clone();
  rs->setText("rs");
  m_ui->tableWidget->setItem(1, 13, rs);
  m_ui->tableWidget->setSpan(1, 13, 1, 5);
  //  funct3
  for (int i = 0; i < 3; ++i) {
    m_ui->tableWidget->setItem(1, i + 18, zeroLightGreen->clone());
  }
  //  rd
  m_ui->tableWidget->setItem(1, 21, rd->clone());
  m_ui->tableWidget->setSpan(1, 21, 1, 5);
  //  opcode
  m_ui->tableWidget->setItem(1, 26, zeroBrightGreen->clone());
  m_ui->tableWidget->setItem(1, 27, zeroBrightGreen->clone());
  m_ui->tableWidget->setItem(1, 28, oneBrightGreen->clone());
  m_ui->tableWidget->setItem(1, 29, zeroBrightGreen->clone());
  m_ui->tableWidget->setItem(1, 30, zeroBrightGreen->clone());
  //  quadrant
  m_ui->tableWidget->setItem(1, 31, oneWhite->clone());
  m_ui->tableWidget->setItem(1, 32, oneWhite->clone());
  // addi immediate
  m_ui->tableWidget->setItem(2, 1, m1ExtendedGreen->clone());
  m_ui->tableWidget->setSpan(2, 1, 1, 21);
  m_ui->tableWidget->setItem(2, 22, imm1Green->clone());
  m_ui->tableWidget->setSpan(2, 22, 1, 11);
}

SliderulesTab::~SliderulesTab() {
  delete m_ui;
}

}
