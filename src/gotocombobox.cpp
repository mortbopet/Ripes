#include "gotocombobox.h"
#include "addressdialog.h"
#include "defines.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>

GoToComboBox::GoToComboBox(QWidget* parent) : QComboBox(parent) {
    // Add default values to goto combobox. Order matters!
    addItem("Select", 0);
    addItem("Address...", 0);
    addItem("Text", 0);
    addItem("Data", DATASTART);
    addItem("Stack", STACKSTART);

    connect(this, QOverload<int>::of(&GoToComboBox::activated), this, &GoToComboBox::signalFilter);
}

void GoToComboBox::signalFilter(int index) {
    switch (index) {
        case 0:
            return;  // "select" has been clicked
        case 1: {
            // Create goto-address dialog
            AddressDialog dialog;
            if (dialog.exec() == QDialog::Accepted) {
                emit jumpToAddress(dialog.getAddress());
            }
            break;
        }
        default:
            // All other indexes should just emit their data - which is their target
            // address
            emit jumpToAddress(itemData(index).toUInt());
            break;
    }

    // revert selection to "Select"
    blockSignals(true);
    setCurrentIndex(0);
    blockSignals(false);
}
