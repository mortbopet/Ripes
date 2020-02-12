#pragma once

#include <QDialog>
#include <QTreeWidget>

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

    ProcessorID getSelectedId() const { return m_selectedID; }
    RegisterInitialization getRegisterInitialization() const;
    Layout getSelectedLayout() const;

private slots:
    void selectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    enum ProcessorTreeColums { ProcessorColumn, ColumnCount };
    ProcessorID m_selectedID;
    Ui::ProcessorSelectionDialog* m_ui;
};
}  // namespace Ripes
