#include "gotocombobox.h"
#include "addressdialog.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QVariant>

#include "processorhandler.h"

namespace Ripes {

GoToComboBox::GoToComboBox(QWidget *parent) : QComboBox(parent) {
  connect(this, QOverload<int>::of(&GoToComboBox::activated), this,
          &GoToComboBox::signalFilter);
}

void GoToComboBox::showPopup() {
  if (count()) {
    clear();
  }

  addItem("Select",
          QVariant::fromValue<GoToUserData>({GoToFunction::Select, 0}));
  addTargets();

  QComboBox::showPopup();
}

void GoToComboBox::signalFilter(int index) {
  const auto &value = itemData(index);

  GoToUserData data;
  if (value.metaType() == QMetaType::fromType<GoToRegisterValue>()) {
    data = qvariant_cast<GoToRegisterValue>(value);
  } else {
    data = qvariant_cast<GoToUserData>(value);
  }
  switch (data.func) {
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
  addItem("Address...",
          QVariant::fromValue<GoToUserData>({GoToFunction::Address, 0}));
  if (auto prog_spt = ProcessorHandler::getProgram()) {
    for (const auto &section : prog_spt->sections) {
      addItem(section.first,
              QVariant::fromValue<GoToUserData>({GoToFunction::Custom, 0}));
    }
  }
}

AInt GoToSectionComboBox::addrForIndex(int i) {
  const QString &sectionName = itemText(i);
  if (auto prog_spt = ProcessorHandler::getProgram()) {
    return prog_spt->getSection(sectionName)->address;
  } else {
    return 0;
  }
}

void GoToRegisterComboBox::addTargets() {
  const auto &isa = ProcessorHandler::currentISA();
  for (const auto &regInfo : isa->regInfos()) {
    for (unsigned i = 0; i < regInfo->regCnt(); ++i) {
      addItem(regInfo->regName(i) + " (" + regInfo->regAlias(i) + ")",
              QVariant::fromValue<GoToRegisterValue>(
                  {{GoToFunction::Custom, i}, regInfo->regFileName()}));
    }
  }
}

AInt GoToRegisterComboBox::addrForIndex(int i) {
  const auto &regIdx = qvariant_cast<GoToRegisterValue>(itemData(i));
  return ProcessorHandler::getRegisterValue(regIdx.regFileName, regIdx.arg);
}

} // namespace Ripes
