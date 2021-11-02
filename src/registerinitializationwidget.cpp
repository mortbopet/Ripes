#include "registerinitializationwidget.h"
#include "ui_registerinitializationwidget.h"

#include "radix.h"

#include <QComboBox>

namespace Ripes {

std::map<ProcessorID, RegisterInitialization> RegisterInitializationWidget::m_initializations;

RegisterSelectionComboBox::RegisterSelectionComboBox(RegisterInitializationWidget* parent)
    : QComboBox(parent), m_parent(parent) {
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] {
        const int oldIdx = m_index;
        m_index = currentData().toUInt();
        emit regIndexChanged(oldIdx, m_index);
    });
}

void RegisterSelectionComboBox::showPopup() {
    if (count())
        clear();

    const auto& procisa = ProcessorRegistry::getAvailableProcessors().at(m_parent->m_currentID)->isaInfo();
    const auto& initializations = m_parent->m_initializations.at(m_parent->m_currentID);

    std::set<unsigned> regOptions;
    for (unsigned i = 0; i < procisa.isa->regCnt(); ++i) {
        if (!procisa.isa->regIsReadOnly(i)) {
            regOptions.insert(i);
        }
    }

    for (const auto& init : initializations) {
        regOptions.erase(init.first);
    }

    for (const auto& i : regOptions) {
        addItem(procisa.isa->regName(i) + " (" + procisa.isa->regAlias(i) + ")", i);
    }
    QComboBox::showPopup();
}

RegisterInitializationWidget::RegisterInitializationWidget(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::RegisterInitializationWidget) {
    m_ui->setupUi(this);

    m_hexValidator = new QRegExpValidator(this);
    m_hexValidator->setRegExp(hexRegex32);

    const QIcon addIcon = QIcon(":/icons/plus.svg");
    m_ui->addInitButton->setIcon(addIcon);
    connect(m_ui->addInitButton, &QPushButton::clicked,
            [=] { this->addRegisterInitialization(getNonInitializedRegIdx()); });

    // Initialize initializations for all available processors
    for (const auto& desc : ProcessorRegistry::getAvailableProcessors()) {
        // Set to initial value if this is the first time the dialog is loaded. Else, keep whatever changes has been
        // stored in the static variable, made in previous invokations of the dialog.
        if (!m_initializations.count(desc.second->id)) {
            m_initializations[desc.second->id] = desc.second->defaultRegisterVals;
        }
    }
}

void RegisterInitializationWidget::processorSelectionChanged(ProcessorID id) {
    // Clear current initialization widgets and recreate initialization widgets based on the currently selected
    // processor
    m_currentRegInitWidgets.clear();
    m_currentID = id;

    for (const auto& init : m_initializations.at(id)) {
        addRegisterInitialization(init.first);
    }

    updateAddButtonState();
}

RegisterInitializationWidget::~RegisterInitializationWidget() {
    delete m_ui;
}

void RegisterInitializationWidget::updateAddButtonState() {
    // Disable add button if we have exhausted the number of registers to initialize for the given processor
    if (getNonInitializedRegIdx() != -1) {
        m_ui->addInitButton->setEnabled(true);
    } else {
        m_ui->addInitButton->setEnabled(false);
    }
}

int RegisterInitializationWidget::getNonInitializedRegIdx() {
    const auto& currentISA = ProcessorRegistry::getAvailableProcessors().at(m_currentID)->isaInfo();
    const auto& currentInitForProc = m_initializations.at(m_currentID);
    unsigned id = 0;
    while (currentInitForProc.count(id) || currentISA.isa->regIsReadOnly(id)) {
        id++;
    }

    return id < currentISA.isa->regCnt() ? id : -1;
}

RegisterInitializationWidget::RegInitWidgets* RegisterInitializationWidget::addRegisterInitialization(unsigned regIdx) {
    constexpr unsigned s_defaultval = 0;
    if (!m_initializations.at(m_currentID).count(regIdx)) {
        // No default value of the register initialization exists.
        m_initializations.at(m_currentID)[regIdx] = s_defaultval;
    }

    const auto& regLayout = m_ui->regInitLayout;
    const auto& procisa = ProcessorRegistry::getAvailableProcessors().at(m_currentID)->isaInfo();

    auto& w = m_currentRegInitWidgets.emplace_back(std::make_unique<RegInitWidgets>());
    auto* w_ptr = w.get();

    w->name = new RegisterSelectionComboBox(this);
    w->value = new QLineEdit(this);
    w->remove = new QPushButton(this);

    w->name->addItem(procisa.isa->regName(regIdx) + " (" + procisa.isa->regAlias(regIdx) + ")", regIdx);

    connect(w->name, &RegisterSelectionComboBox::regIndexChanged, [this, w_ptr](int oldIdx, int newIdx) {
        m_initializations.at(m_currentID).erase(oldIdx);
        m_initializations.at(m_currentID)[newIdx];
        emit w_ptr->value->textChanged(w_ptr->value->text());
    });

    const QIcon removeIcon = QIcon(":/icons/delete.svg");
    w->remove->setIcon(removeIcon);

    w->value->setValidator(m_hexValidator);
    w->value->setText("0x" + QString::number(m_initializations.at(m_currentID).at(regIdx), 16));
    connect(w->value, &QLineEdit::textChanged, [this, w_ptr](const QString& text) {
        this->m_initializations.at(this->m_currentID).at(w_ptr->name->currentData().toUInt()) =
            text.toUInt(nullptr, 16);
    });

    const int nInitializations = regLayout->rowCount();
    regLayout->addWidget(w->name, nInitializations, 0);
    regLayout->addWidget(w->value, nInitializations, 1);
    regLayout->addWidget(w->remove, nInitializations, 2);
    connect(w->remove, &QPushButton::clicked, [this, w_ptr] { this->removeRegInitWidget(w_ptr); });

    updateAddButtonState();
    return w.get();
}

void RegisterInitializationWidget::removeRegInitWidget(RegisterInitializationWidget::RegInitWidgets* w) {
    auto iter = std::find_if(m_currentRegInitWidgets.begin(), m_currentRegInitWidgets.end(),
                             [=](const auto& initWidget) { return initWidget.get() == w; });
    assert(iter != m_currentRegInitWidgets.end());

    // Current register index stored in the combobox of the regInitWidgets
    const unsigned regIdx = w->name->itemData(0).toUInt();
    m_initializations.at(m_currentID).erase(regIdx);

    m_currentRegInitWidgets.erase(iter);
    updateAddButtonState();
}

void RegisterInitializationWidget::RegInitWidgets::clear() {
    delete name;
    delete value;
    delete remove;
}

RegisterInitialization RegisterInitializationWidget::getInitialization() const {
    return m_initializations.at(m_currentID);
}

}  // namespace Ripes
