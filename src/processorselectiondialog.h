#pragma once

#include <QDialog>
#include <QListWidget>

#include "processorhandler.h"
#include "processorregistry.h"

namespace Ui {
class ProcessorSelectionDialog;
}

class ProcessorSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProcessorSelectionDialog(const ProcessorHandler& handler, QWidget* parent = nullptr);
    ~ProcessorSelectionDialog();

    ProcessorID selectedID;

public slots:
    virtual void accept() override;

private slots:
    void selectionChanged(QListWidgetItem* current, QListWidgetItem* previous);

private:
    Ui::ProcessorSelectionDialog* ui;

    const ProcessorHandler& m_handler;
};
