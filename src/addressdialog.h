#pragma once

#include <QDialog>

#include "ripes_types.h"

namespace Ripes {

namespace Ui {
class AddressDialog;
}

class AddressDialog : public QDialog {
    Q_OBJECT

public:
    friend class GoToComboBox;
    explicit AddressDialog(QWidget* parent = nullptr);
    ~AddressDialog() override;

    AInt getAddress() const { return m_address; }

private:
    Ui::AddressDialog* m_ui = nullptr;
    AInt m_address = 0;
    void validateTargetAddress(const QString& address);
};
}  // namespace Ripes
