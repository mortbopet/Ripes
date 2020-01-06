#include "processorselectiondialog.h"
#include "ui_processorselectiondialog.h"

#include <QDialogButtonBox>
#include <QListWidget>

#include "processorregistry.h"

ProcessorSelectionDialog::ProcessorSelectionDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::ProcessorSelectionDialog) {
    ui->setupUi(this);
    setWindowTitle("Select Processor");

    // Initialize processor list
    for (auto& desc : ProcessorRegistry::getAvailableProcessors()) {
        QListWidgetItem* item = new QListWidgetItem(desc.second.name);
        item->setData(Qt::UserRole, QVariant::fromValue(desc.second.id));
        if (desc.second.id == ProcessorRegistry::currentProcessorID()) {
            auto font = item->font();
            font.setBold(true);
            item->setFont(font);
        }
        ui->processorList->addItem(item);
    }

    connect(ui->processorList, &QListWidget::currentItemChanged, this, &ProcessorSelectionDialog::selectionChanged);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ProcessorSelectionDialog::~ProcessorSelectionDialog() {
    delete ui;
}

void ProcessorSelectionDialog::accept() {
    const ProcessorID id = qvariant_cast<ProcessorID>(ui->processorList->currentItem()->data(Qt::UserRole));
    if (ProcessorRegistry::selectProcessor(id)) {
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
    ui->ISA->setText(desc.ISA);
    ui->description->setPlainText(desc.description);
}
