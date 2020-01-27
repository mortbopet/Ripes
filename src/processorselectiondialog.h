#pragma once

#include <QDialog>
#include <QListWidget>

#include "processorregistry.h"

namespace Ripes {

namespace Ui {
class ProcessorSelectionDialog;
}

class ProcessorSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProcessorSelectionDialog(QWidget* parent = nullptr);
    ~ProcessorSelectionDialog();

    ProcessorID getSelectedId() const { return selectedID; }
    RegisterInitialization getRegisterInitialization() const;
    Layout getSelectedLayout() const;

public slots:
    virtual void accept() override;

private slots:
    void selectionChanged(QListWidgetItem* current, QListWidgetItem* previous);

private:
    ProcessorID selectedID;
    Ui::ProcessorSelectionDialog* ui;
};
}  // namespace Ripes
