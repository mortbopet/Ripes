#ifndef ADDRESSDIALOG_H
#define ADDRESSDIALOG_H

#include <QDialog>

namespace Ui {
class AddressDialog;
}

class AddressDialog : public QDialog {
    Q_OBJECT

public:
    friend class GoToComboBox;
    explicit AddressDialog(QWidget* parent = nullptr);
    ~AddressDialog() override;

    uint32_t getAddress() const { return m_address; }

private:
    Ui::AddressDialog* m_ui;
    uint32_t m_address = 0;
    void validateTargetAddress(const QString& address);
};

#endif  // ADDRESSDIALOG_H
