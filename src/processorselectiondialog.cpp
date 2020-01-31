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
    QListWidgetItem* selectedItem = nullptr;
    for (auto& desc : ProcessorRegistry::getAvailableProcessors()) {
        QListWidgetItem* item = new QListWidgetItem(desc.second.name);
        item->setData(Qt::UserRole, QVariant::fromValue(desc.second.id));
        if (desc.second.id == ProcessorHandler::get()->getID()) {
            auto font = item->font();
            font.setBold(true);
            item->setFont(font);
            selectedItem = item;
        }
        ui->processorList->addItem(item);
    }

    connect(ui->processorList, &QListWidget::currentItemChanged, this, &ProcessorSelectionDialog::selectionChanged);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (selectedItem != nullptr) {
        ui->processorList->setCurrentItem(selectedItem);
    }
}

RegisterInitialization ProcessorSelectionDialog::getRegisterInitialization() const {
    return ui->regInitWidget->getInitialization();
}

ProcessorSelectionDialog::~ProcessorSelectionDialog() {
    delete ui;
}

void ProcessorSelectionDialog::accept() {
    const ProcessorID id = qvariant_cast<ProcessorID>(ui->processorList->currentItem()->data(Qt::UserRole));
    selectedID = id;
    QDialog::accept();
}

void ProcessorSelectionDialog::selectionChanged(QListWidgetItem* current, QListWidgetItem* previous) {
    const ProcessorID id = qvariant_cast<ProcessorID>(current->data(Qt::UserRole));
    const auto& desc = ProcessorRegistry::getAvailableProcessors().at(id);

    ui->name->setText(desc.name);
    ui->ISA->setText(desc.isa->name());
    ui->description->setPlainText(desc.description);
    ui->regInitWidget->processorSelectionChanged(id);

    ui->layout->clear();
    for (const auto& layout : desc.layouts) {
        ui->layout->addItem(layout.name);
    }
}

Layout ProcessorSelectionDialog::getSelectedLayout() const {
    const auto& desc = ProcessorRegistry::getAvailableProcessors().at(getSelectedId());
    for (const auto& layout : desc.layouts) {
        if (layout.name == ui->layout->currentText()) {
            return layout;
        }
    }
    Q_UNREACHABLE();
}

}  // namespace Ripes
