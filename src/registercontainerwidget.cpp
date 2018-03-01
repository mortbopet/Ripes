#include "registercontainerwidget.h"
#include <QScrollBar>
#include "ui_registercontainerwidget.h"

#include "registerwidget.h"

#include "defines.h"

#include <QVBoxLayout>

RegisterContainerWidget::RegisterContainerWidget(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::RegisterContainerWidget) {
    m_ui->setupUi(this);
    // Add display types to display combobox
    for (const auto& type : displayTypes.keys()) {
        m_ui->displayType->insertItem(0, type, displayTypes[type]);
    }
    m_ui->displayType->setCurrentIndex(displayTypeN::Decimal);
}

void RegisterContainerWidget::update() {
    for (auto& reg : m_registers) {
        reg->setText();
    }
}

void RegisterContainerWidget::init() {
    // Initialize register descriptions
    QStringList descriptions = QStringList() << "Hard-Wired zero"
                                             << "Return Address \nSaver: Caller"
                                             << "Stack pointer\nSaver: Callee"
                                             << "Global pointer"
                                             << "Thread pointer"
                                             << "Temporary/alternate link register\nSaver: Caller"
                                             << "Temporary\nSaver: Caller"
                                             << "Temporary\nSaver: Caller"
                                             << "Saved register/frame pointer\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Function argument/return value\nSaver: Caller"
                                             << "Function argument/return value\nSaver: Caller"
                                             << "Function argument\nSaver: Caller"
                                             << "Function argument\nSaver: Caller"
                                             << "Function argument\nSaver: Caller"
                                             << "Function argument\nSaver: Caller"
                                             << "Function argument\nSaver: Caller"
                                             << "Function argument\nSaver: Caller"
                                             << "Saved register\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Saved register\nSaver: Callee"
                                             << "Temporary register\nSaver: Caller"
                                             << "Temporary register\nSaver: Caller"
                                             << "Temporary register\nSaver: Caller"
                                             << "Temporary register\nSaver: Caller";

    // Initialize 32 register widgets
    for (int i = 0; i < 32; i++) {
        auto reg = new RegisterWidget(this);
        if (i == 0) {
            // Disable register 0 (hardwired zero)
            reg->setEnabled(false);
        }
        reg->setRegPtr(&(*m_regPtr)[i]);
        reg->setAlias(ABInames.key(i));
        reg->setNumber(i);
        reg->setToolTip(descriptions[i]);
        reg->setDisplayType(qvariant_cast<displayTypeN>(m_ui->displayType->currentData()));
        connect(m_ui->displayType, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
                [=] { reg->setDisplayType(qvariant_cast<displayTypeN>(m_ui->displayType->currentData())); });
        connect(reg, &RegisterWidget::valueChanged, this, &RegisterContainerWidget::registerHasChanged);
        m_ui->registerLayout->addWidget(reg);
        m_registers.append(reg);
    }
}

void RegisterContainerWidget::registerHasChanged() {
    auto* sender = qobject_cast<RegisterWidget*>(QObject::sender());
    sender->setHighlightState(true);
    if (m_currentHighlightedReg != sender && m_currentHighlightedReg != nullptr) {
        m_currentHighlightedReg->setHighlightState(false);
    }
    m_currentHighlightedReg = sender;

    // Least recently changed register should always be visible in the container
    m_ui->scrollArea->ensureWidgetVisible(sender);
}
