#include "gotocombobox.h"
#include "addressdialog.h"
#include "defines.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QVariant>

#include "processorhandler.h"

GoToComboBox::GoToComboBox(QWidget* parent) : QComboBox(parent) {
    connect(this, QOverload<int>::of(&GoToComboBox::activated), this, &GoToComboBox::signalFilter);
}

void GoToComboBox::showPopup() {
    if (count())
        clear();

    addItem("Select", QVariant::fromValue<GoToFunction>(GoToFunction::Select));
    addItem("Address...", QVariant::fromValue<GoToFunction>(GoToFunction::Address));
    for (const auto& seg : ProcessorHandler::get()->getSetup().segmentPtrs) {
        addItem(s_programSegmentName.at(seg.first), QVariant::fromValue<ProgramSegment>(seg.first));
    }
    QComboBox::showPopup();
}

void GoToComboBox::signalFilter(int index) {
    const auto& value = itemData(index);

    if (value.userType() == QVariant::fromValue(GoToFunction::Select).userType()) {
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
        }
    }

    if (value.userType() == QVariant::fromValue(ProgramSegment::Data).userType()) {
        const ProgramSegment seg = qvariant_cast<ProgramSegment>(value);
        emit jumpToAddress(ProcessorHandler::get()->getSetup().segmentPtrs.at(seg));
    }

    clear();
}
