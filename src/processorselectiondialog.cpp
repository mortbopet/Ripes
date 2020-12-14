#include "processorselectiondialog.h"
#include "ui_processorselectiondialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>

#include "processorhandler.h"
#include "radix.h"
#include "ripessettings.h"

namespace Ripes {

ProcessorSelectionDialog::ProcessorSelectionDialog(QWidget* parent)
    : QDialog(parent), m_ui(new Ui::ProcessorSelectionDialog) {
    m_ui->setupUi(this);
    setWindowTitle("Select Processor");

    // Initialize top level ISA items
    m_ui->processors->setHeaderHidden(true);
    std::map<ISA, QTreeWidgetItem*> isaItems;
    for (const auto& isa : ISAFamilyNames) {
        auto* isaItem = new QTreeWidgetItem({isa.second});
        isaItems[isa.first] = isaItem;
        isaItem->setFlags(isaItem->flags() & ~(Qt::ItemIsSelectable));
        m_ui->processors->insertTopLevelItem(m_ui->processors->topLevelItemCount(), isaItem);
    }

    // Initialize processor list
    QTreeWidgetItem* selectedItem = nullptr;

    for (auto& desc : ProcessorRegistry::getAvailableProcessors()) {
        QTreeWidgetItem* processorItem = new QTreeWidgetItem({desc.second.name});
        processorItem->setData(ProcessorColumn, Qt::UserRole, QVariant::fromValue(desc.second.id));
        if (desc.second.id == ProcessorHandler::get()->getID()) {
            auto font = processorItem->font(ProcessorColumn);
            font.setBold(true);
            processorItem->setFont(ProcessorColumn, font);
            selectedItem = processorItem;
        }
        auto* isaItem = isaItems.at(desc.second.isa->isaID());
        isaItem->insertChild(isaItem->childCount(), processorItem);
    }

    connect(m_ui->processors, &QTreeWidget::currentItemChanged, this, &ProcessorSelectionDialog::selectionChanged);

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Initialize extensions for processors; default to all available extensions
    for (const auto& desc : ProcessorRegistry::getAvailableProcessors()) {
        m_selectedExtensionsForID[desc.second.id] = desc.second.isa->supportedExtensions();
    }
    m_selectedExtensionsForID[ProcessorHandler::get()->getID()] =
        ProcessorHandler::get()->currentISA()->enabledExtensions();

    if (selectedItem != nullptr) {
        // Select the processor and layout which is currently active
        m_ui->processors->setCurrentItem(selectedItem);
        int layoutID = RipesSettings::value(RIPES_SETTING_PROCESSOR_LAYOUT_ID).toInt();
        if (layoutID >= ProcessorRegistry::getDescription(ProcessorHandler::get()->getID()).layouts.size()) {
            layoutID = 0;
        }
        m_ui->layout->setCurrentIndex(layoutID);
    }
}

RegisterInitialization ProcessorSelectionDialog::getRegisterInitialization() const {
    return m_ui->regInitWidget->getInitialization();
}

ProcessorSelectionDialog::~ProcessorSelectionDialog() {
    delete m_ui;
}

QStringList ProcessorSelectionDialog::getEnabledExtensions() const {
    return m_selectedExtensionsForID.at(m_selectedID);
}

void ProcessorSelectionDialog::selectionChanged(QTreeWidgetItem* current, QTreeWidgetItem*) {
    QVariant selectedItemData = current->data(ProcessorColumn, Qt::UserRole);
    const bool validSelection = selectedItemData.canConvert<ProcessorID>();
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validSelection);
    if (!validSelection) {
        // Something which is not a processor was selected (ie. an ISA). Disable OK button
        return;
    }

    const ProcessorID id = qvariant_cast<ProcessorID>(current->data(ProcessorColumn, Qt::UserRole));
    const auto& desc = ProcessorRegistry::getAvailableProcessors().at(id);

    // Update information widgets with the current processor info
    m_selectedID = id;
    m_ui->name->setText(desc.name);
    m_ui->ISA->setText(desc.isa->name());
    m_ui->description->setPlainText(desc.description);
    m_ui->regInitWidget->processorSelectionChanged(id);

    m_ui->layout->clear();
    for (const auto& layout : desc.layouts) {
        m_ui->layout->addItem(layout.name);
    }

    // Setup extensions; Clear previously selected extensions and add whatever extensions are supported for the selected
    // processor
    QLayoutItem* item;
    while ((item = m_ui->extensions->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (const auto& ext : desc.isa->supportedExtensions()) {
        auto chkbox = new QCheckBox(ext);
        m_ui->extensions->addWidget(chkbox);
        if (m_selectedExtensionsForID[desc.id].contains(ext)) {
            chkbox->setChecked(true);
        }
        connect(chkbox, &QCheckBox::toggled, [=](bool toggled) {
            if (toggled) {
                m_selectedExtensionsForID[id] << ext;
            } else {
                m_selectedExtensionsForID[id].removeAll(ext);
            }
        });
    }
}

Layout ProcessorSelectionDialog::getSelectedLayout() const {
    const auto& desc = ProcessorRegistry::getAvailableProcessors().at(m_selectedID);
    for (const auto& layout : desc.layouts) {
        if (layout.name == m_ui->layout->currentText()) {
            return layout;
        }
    }
    Q_UNREACHABLE();
}

}  // namespace Ripes
