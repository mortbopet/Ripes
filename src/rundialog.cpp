#include "rundialog.h"
#include "ui_rundialog.h"

#include "pipeline.h"
#include "processortab.h"

#include <QPushButton>
#include <QtConcurrent/QtConcurrent>

RunDialog::RunDialog(QWidget* parent) : QDialog(parent), ui(new Ui::RunDialog) {
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Abort");

    m_processorTab = static_cast<ProcessorTab*>(parent);

    setWindowTitle("Running...");

    ui->label->setText(QString("Running for 0 seconds"));
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &RunDialog::updateText);
    m_timer.start();
}

void RunDialog::updateText() {
    runtime++;
    ui->label->setText(QString("Running for %1 seconds").arg(runtime));
    if (runtime > 15) {
        ui->extra->setText(QString("Your program may contain an infinite loop"));
    }
}
QFuture<int> RunDialog::startSimulation() {
    // Ensure that abort flag of pipeline is cleared before running again
    Pipeline::getPipeline()->clearAbort();
    QFuture<int> future = QtConcurrent::run(Pipeline::getPipeline(), &Pipeline::run);
    return future;
}

int RunDialog::exec() {
    connect(&m_runWatcher, &QFutureWatcher<int>::finished, this, &RunDialog::finished);

    m_runWatcher.setFuture(startSimulation());
    return QDialog::exec();
}

RunDialog::~RunDialog() {
    delete ui;
}

void RunDialog::finished() {
    if (m_runWatcher.future().result() == 0) {
        m_timer.stop();
        accept();
    } else {
        // Check if there was an ecall
        const auto ecall_val = Pipeline::getPipeline()->checkEcall(true);
        if (ecall_val.first != Pipeline::ECALL::none) {
            // An ECALL has been invoked during continuous running. Handle ecall and continue to run
            m_processorTab->handleEcall(ecall_val);
            // Check whether to stop the pipeline given an ecall::exit instruction
            if (!(Pipeline::getPipeline()->isFinished() || ecall_val.first == Pipeline::ECALL::exit)) {
                m_runWatcher.setFuture(startSimulation());
            } else {
                accept();
            }
        }
    }
}

void RunDialog::on_buttonBox_clicked(QAbstractButton* button) {
    if (button == ui->buttonBox->button(QDialogButtonBox::Cancel)) {
        // Disable the futureWatcher as to not call stateChanged twice
        Pipeline::getPipeline()->abort();
    }
}
