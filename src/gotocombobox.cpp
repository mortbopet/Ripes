#include "gotocombobox.h"
#include "addressdialog.h"
#include "defines.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QVariant>

#include "processorhandler.h"

namespace Ripes {

GoToComboBox::GoToComboBox(QWidget* parent) : QComboBox(parent) {
    connect(this, QOverload<int>::of(&GoToComboBox::activated), this, &GoToComboBox::signalFilter);
}

void GoToComboBox::showPopup() {
    if (count())
        clear();

    addItem("Select", QVariant::fromValue<GoToFunction>(GoToFunction::Select));
    addItem("Address...", QVariant::fromValue<GoToFunction>(GoToFunction::Address));

    if (ProcessorHandler::get()->getProgram()) {
        for (const auto& section : ProcessorHandler::get()->getProgram()->sections) {
            addItem(section.name, QVariant::fromValue<GoToFunction>(GoToFunction::Section));
        }
    }

    QComboBox::showPopup();
}

void GoToComboBox::signalFilter(int index) {
    const auto& value = itemData(index);

    const GoToFunction f = qvariant_cast<GoToFunction>(value);
    switch (f) {
        case GoToFunction::Select:
            return;
        case GoToFunction::Address: {
            // Create goto-address dialog
            AddressDialog dialog;
            if (dialog.exec() == QDialog::Accepted) {
                emit jumpToAddress(dialog.getAddress());
            }
            break;
        }
        case GoToFunction::Section: {
            const QString& sectionName = itemText(index);
            emit jumpToAddress(ProcessorHandler::get()->getProgram()->getSection(sectionName)->address);
            break;
        }
    }

    clear();
}
}  // namespace Ripes
