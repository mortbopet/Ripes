#include "processorselectiondialog.h"
#include "ui_processorselectiondialog.h"

#include <QDialogButtonBox>
#include <QListWidget>

#include "processorhandler.h"
#include "radix.h"

namespace Ripes {

ProcessorSelectionDialog::ProcessorSelectionDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::ProcessorSelectionDialog) {
    ui->setupUi(this);
    setWindowTitle("Select Processor");

    // Initialize processor list
    for (auto& desc : ProcessorRegistry::getAvailableProcessors()) {
        QListWidgetItem* item = new QListWidgetItem(desc.second.name);
        item->setData(Qt::UserRole, QVariant::fromValue(desc.second.defaultSetup.id));
        if (desc.second.defaultSetup.id == ProcessorHandler::get()->getSetup().id) {
            auto font = item->font();
            font.setBold(true);
            item->setFont(font);
        }
        ui->processorList->addItem(item);
    }

    QRegExpValidator* validator = new QRegExpValidator(this);
    validator->setRegExp(hexRegex32);
    ui->stackPtr->setValidator(validator);
    ui->dataPtr->setValidator(validator);
    connect(ui->processorList, &QListWidget::currentItemChanged, this, &ProcessorSelectionDialog::selectionChanged);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ProcessorSelectionDialog::~ProcessorSelectionDialog() {
    delete ui;
}

void ProcessorSelectionDialog::accept() {
    const ProcessorID id = qvariant_cast<ProcessorID>(ui->processorList->currentItem()->data(Qt::UserRole));
    selectedSetup = ProcessorRegistry::getDescription(id).defaultSetup;

    selectedSetup.segmentPtrs[ProgramSegment::Data] = ui->dataPtr->text().toUInt(nullptr, 16);
    selectedSetup.segmentPtrs[ProgramSegment::Stack] = ui->stackPtr->text().toUInt(nullptr, 16);

    if (id != ProcessorHandler::get()->getSetup().id) {
        // New processor model was selected
        return QDialog::accept();
    } else {
        return QDialog::reject();
    }
}

void ProcessorSelectionDialog::selectionChanged(QListWidgetItem* current, QListWidgetItem* previous) {
    const ProcessorID id = qvariant_cast<ProcessorID>(current->data(Qt::UserRole));
    const auto& desc = ProcessorRegistry::getAvailableProcessors().at(id);

    ui->name->setText(desc.name);
    ui->ISA->setText(desc.isa->name());
    ui->description->setPlainText(desc.description);

    ui->stackPtr->setText(
        "0x" +
        QString::number(ProcessorRegistry::getDescription(id).defaultSetup.segmentPtrs.at(ProgramSegment::Stack), 16));
    ui->dataPtr->setText(
        "0x" +
        QString::number(ProcessorRegistry::getDescription(id).defaultSetup.segmentPtrs.at(ProgramSegment::Data), 16));
}
}  // namespace Ripes
