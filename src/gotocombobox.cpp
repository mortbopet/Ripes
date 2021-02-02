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

    addItem("Select", QVariant::fromValue<GoToUserData>({GoToFunction::Select, 0}));
    addTargets();

    QComboBox::showPopup();
}

void GoToComboBox::signalFilter(int index) {
    const auto& value = itemData(index);

    const auto f = qvariant_cast<GoToUserData>(value);
    switch (f.func) {
        case GoToFunction::Select:
            break;
        case GoToFunction::Address: {
            // Create goto-address dialog
            AddressDialog dialog;
            if (dialog.exec() == QDialog::Accepted) {
                emit jumpToAddress(dialog.getAddress());
            }
            break;
        }
        case GoToFunction::Custom: {
            emit jumpToAddress(addrForIndex(index));
            break;
        }
    }
    clear();
}

void GoToSectionComboBox::addTargets() {
    addItem("Address...", QVariant::fromValue<GoToUserData>({GoToFunction::Address, 0}));
    if (auto prog_spt = ProcessorHandler::get()->getProgram()) {
        for (const auto& section : prog_spt->sections) {
            addItem(section.first, QVariant::fromValue<GoToUserData>({GoToFunction::Custom, 0}));
        }
    }
}

uint32_t GoToSectionComboBox::addrForIndex(int i) {
    const QString& sectionName = itemText(i);
    if (auto prog_spt = ProcessorHandler::get()->getProgram()) {
        return prog_spt->getSection(sectionName)->address;
    } else {
        return -1;
    }
}

void GoToRegisterComboBox::addTargets() {
    const auto& isa = ProcessorHandler::get()->currentISA();
    for (unsigned i = 0; i < isa->regCnt(); i++) {
        addItem(isa->regName(i) + " (" + isa->regAlias(i) + ")",
                QVariant::fromValue<GoToUserData>({GoToFunction::Custom, i}));
    }
}

uint32_t GoToRegisterComboBox::addrForIndex(int i) {
    const auto& data = qvariant_cast<GoToUserData>(itemData(i));
    return ProcessorHandler::get()->getRegisterValue(RegisterFileType::GPR, data.arg);
}

}  // namespace Ripes
