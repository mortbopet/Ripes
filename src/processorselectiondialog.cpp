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
    std::map<QString, QTreeWidgetItem*> isaFamilyItems;
    std::map<QTreeWidgetItem*, std::map<unsigned, QTreeWidgetItem*>> isaFamilyRegWidthItems;
    for (const auto& isa : ISAFamilyNames) {
        if (isaFamilyItems.count(isa.second) == 0) {
            isaFamilyItems[isa.second] = new QTreeWidgetItem({isa.second});
        }
        auto* isaItem = isaFamilyItems.at(isa.second);
        isaFamilyRegWidthItems[isaItem] = {};
        isaItem->setFlags(isaItem->flags() & ~(Qt::ItemIsSelectable));
        m_ui->processors->insertTopLevelItem(m_ui->processors->topLevelItemCount(), isaItem);
    }

    // Initialize processor list
    QTreeWidgetItem* selectedItem = nullptr;

    for (auto& desc : ProcessorRegistry::getAvailableProcessors()) {
        QTreeWidgetItem* processorItem = new QTreeWidgetItem({desc.second->name});
        processorItem->setData(ProcessorColumn, Qt::UserRole, QVariant::fromValue(desc.second->id));
        if (desc.second->id == ProcessorHandler::getID()) {
            // Highlight if currently selected processor
            auto font = processorItem->font(ProcessorColumn);
            font.setBold(true);
            processorItem->setFont(ProcessorColumn, font);
            selectedItem = processorItem;
        }
        const QString& isaFamily = ISAFamilyNames.at(desc.second->isaInfo().isa->isaID());
        QTreeWidgetItem* familyItem = isaFamilyItems.at(isaFamily);
        auto& regWidthItemsForISA = isaFamilyRegWidthItems.at(familyItem);
        auto isaRegWidthItem = regWidthItemsForISA.find(desc.second->isaInfo().isa->bits());
        if (isaRegWidthItem == regWidthItemsForISA.end()) {
            // Create reg width item
            auto* widthItem = new QTreeWidgetItem({QString::number(desc.second->isaInfo().isa->bits()) + "-bit"});
            widthItem->setFlags(widthItem->flags() & ~(Qt::ItemIsSelectable));
            regWidthItemsForISA[desc.second->isaInfo().isa->bits()] = widthItem;
            isaRegWidthItem = regWidthItemsForISA.find(desc.second->isaInfo().isa->bits());
            familyItem->insertChild(familyItem->childCount(), widthItem);
        }
        isaRegWidthItem->second->insertChild(isaRegWidthItem->second->childCount(), processorItem);
    }

    connect(m_ui->processors, &QTreeWidget::currentItemChanged, this, &ProcessorSelectionDialog::selectionChanged);
    connect(m_ui->processors, &QTreeWidget::itemDoubleClicked, this, [=](const QTreeWidgetItem* item) {
        if (isCPUItem(item)) {
            accept();
        }
    });

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Initialize extensions for processors; default to all available extensions
    for (const auto& desc : ProcessorRegistry::getAvailableProcessors()) {
        m_selectedExtensionsForID[desc.second->id] = desc.second->isaInfo().defaultExtensions;
    }
    m_selectedExtensionsForID[ProcessorHandler::getID()] = ProcessorHandler::currentISA()->enabledExtensions();

    if (selectedItem != nullptr) {
        // Select the processor and layout which is currently active
        m_ui->processors->setCurrentItem(selectedItem);
        unsigned layoutID = RipesSettings::value(RIPES_SETTING_PROCESSOR_LAYOUT_ID).toInt();
        if (layoutID >= ProcessorRegistry::getDescription(ProcessorHandler::getID()).layouts.size()) {
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

bool ProcessorSelectionDialog::isCPUItem(const QTreeWidgetItem* item) const {
    if (!item) {
        return false;
    }
    QVariant selectedItemData = item->data(ProcessorColumn, Qt::UserRole);
    const bool validSelection = selectedItemData.canConvert<ProcessorID>();
    return validSelection;
}

void ProcessorSelectionDialog::selectionChanged(QTreeWidgetItem* current, QTreeWidgetItem*) {
    if (current == nullptr) {
        return;
    }

    const bool validSelection = isCPUItem(current);
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validSelection);
    if (!validSelection) {
        // Something which is not a processor was selected (ie. an ISA). Disable OK button
        return;
    }

    const ProcessorID id = qvariant_cast<ProcessorID>(current->data(ProcessorColumn, Qt::UserRole));
    const auto& desc = ProcessorRegistry::getAvailableProcessors().at(id);
    auto isaInfo = desc->isaInfo();

    // Update information widgets with the current processor info
    m_selectedID = id;
    m_ui->name->setText(desc->name);
    m_ui->ISA->setText(isaInfo.isa->name());
    m_ui->description->setPlainText(desc->description);
    m_ui->regInitWidget->processorSelectionChanged(id);

    m_ui->layout->clear();
    for (const auto& layout : desc->layouts) {
        m_ui->layout->addItem(layout.name);
    }

    // Setup extensions; Clear previously selected extensions and add whatever extensions are supported for the selected
    // processor
    QLayoutItem* item;
    while ((item = m_ui->extensions->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (const auto& ext : isaInfo.supportedExtensions) {
        auto chkbox = new QCheckBox(ext);
        chkbox->setToolTip(isaInfo.isa->extensionDescription(ext));
        m_ui->extensions->addWidget(chkbox);
        if (m_selectedExtensionsForID[desc->id].contains(ext)) {
            chkbox->setChecked(true);
        }
        connect(chkbox, &QCheckBox::toggled, this, [=](bool toggled) {
            if (toggled) {
                m_selectedExtensionsForID[id] << ext;
            } else {
                m_selectedExtensionsForID[id].removeAll(ext);
            }
        });
    }
}

const Layout* ProcessorSelectionDialog::getSelectedLayout() const {
    const auto& desc = ProcessorRegistry::getAvailableProcessors().at(m_selectedID);
    for (const auto& layout : desc->layouts) {
        if (layout.name == m_ui->layout->currentText()) {
            return &layout;
        }
    }
    return nullptr;
    Q_UNREACHABLE();
}

}  // namespace Ripes
