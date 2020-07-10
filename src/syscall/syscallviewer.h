#pragma once
#include <QDialog>
#include <QTableWidgetItem>

#include "ripes_syscall.h"

namespace Ripes {

namespace Ui {
class SyscallViewer;
}

class SyscallViewer : public QDialog {
    Q_OBJECT

public:
    explicit SyscallViewer(QWidget* parent = nullptr);
    ~SyscallViewer();

private:
    void handleItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
    void addSyscall(unsigned id, const Syscall* syscall);
    void addItemToTable(QTableWidget* table, unsigned idx, const QString& description);
    void setCurrentSyscall(const Syscall* syscall);

    Ui::SyscallViewer* m_ui;
};
}  // namespace Ripes
