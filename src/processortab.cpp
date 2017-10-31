#include "processortab.h"
#include "ui_processortab.h"

ProcessorTab::ProcessorTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProcessorTab)
{
    ui->setupUi(this);
}

ProcessorTab::~ProcessorTab()
{
    delete ui;
}
