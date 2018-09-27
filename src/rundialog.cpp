#include "rundialog.h"
#include "ui_rundialog.h"

#include "pipeline.h"

#include <QPushButton>
#include <QtConcurrent/QtConcurrent>

RunDialog::RunDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Abort");

    setWindowTitle("Running...");

    ui->label->setText(QString("Running for 0 seconds"));
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &RunDialog::updateText);
    m_timer.start();
}

void RunDialog::updateText(){
    runtime++;
    ui->label->setText(QString("Running for %1 seconds").arg(runtime));
    if(runtime > 15){
        ui->extra->setText(QString("Your program may contain an infinite loop"));
    }
}
QFuture<int> RunDialog::startSimulation(){
    QFuture<int> future = QtConcurrent::run(Pipeline::getPipeline(), &Pipeline::run);
    return future;
}

int RunDialog::exec(){
    connect(&m_runWatcher, &QFutureWatcher<int>::finished,
            this, &RunDialog::finished);

    m_runWatcher.setFuture(startSimulation());
    return QDialog::exec();
}

RunDialog::~RunDialog()
{
    delete ui;
}

void RunDialog::finished(){
    m_timer.stop();
    if(m_runWatcher.future().result() == 0){
        accept();
    } else {
        reject();
    }
}

void RunDialog::on_buttonBox_clicked(QAbstractButton* button) {
    if (button == ui->buttonBox->button(QDialogButtonBox::Cancel)) {
        // Disable the futureWatcher as to not call stateChanged twice
        Pipeline::getPipeline()->abort();
    }
}
