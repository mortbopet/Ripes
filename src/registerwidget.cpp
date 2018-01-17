#include "registerwidget.h"
#include "ui_registerwidget.h"

RegisterWidget::RegisterWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::RegisterWidget) {
    m_ui->setupUi(this);
    // m_ui->value->setValidator(&m_validator);

    connect(m_ui->value, &QLineEdit::editingFinished, this, &RegisterWidget::validateInput);
}

RegisterWidget::~RegisterWidget() {
    delete m_ui;
}

void RegisterWidget::setAlias(QString text) {
    m_ui->alias->setText(text);
}

void RegisterWidget::validateInput() {
    // Instead of subclassing QValidator for ie. hex, we do some simple input
    // validation here
    const QString input = m_ui->value->text();

    bool ok;
    auto value = input.toLongLong(&ok, m_displayBase);
    if (ok && value >= m_range.first && value <= m_range.second) {  // verify "ok" and that value is within current
                                                                    // accepted range of the display type
        *m_regPtr = value;
    } else {
        // revert lineedit to the current register value
        setText();
    }
}

void RegisterWidget::setText() {
    // Sets line edit text based on current display type and register value
    QString newValue;
    if (m_displayType == displayTypeN::Hex) {
        // hex
        newValue = QString().setNum(*m_regPtr, 16).rightJustified(8, '0');  // zero padding on hex numbers
    } else if (m_displayType == displayTypeN::Binary) {
        // binary
        newValue = QString().setNum(*m_regPtr, 2);
    } else if (m_displayType == displayTypeN::Decimal) {
        // Decimal
        newValue = QString().setNum(*(int32_t*)m_regPtr, 10);
    } else if (m_displayType == displayTypeN::Unsigned) {
        // Unsigned
        newValue = QString().setNum(*m_regPtr, 10);
    } else if (m_displayType == displayTypeN::ASCII) {
        // ASCII - valid ascii characters will not be able to fill the 32-bit
        // range
        QString out;
        auto value = *m_regPtr;
        for (int i = 0; i < 4; i++) {
            out.append(QChar(value & 0xff));
            value >>= 8;
        }
        newValue = out;
        m_ui->value->setInputMask("nnnn");
    }

    // RegisterContainerWidget will set the background color for the most recently edited register
    if (m_ui->value->text() != newValue) {
        emit valueChanged();
    }
    m_ui->value->setText(newValue);
}

void RegisterWidget::setHighlightState(bool state) {
    if (state) {
        QPalette palette;
        palette.setColor(QPalette::Base, QColor(Colors::Medalist).lighter(200));
        m_ui->value->setPalette(palette);
    } else {
        m_ui->value->setPalette(QPalette());
    }
}

void RegisterWidget::setNumber(int number) {
    m_ui->number->setText(QString("x(%1)").arg(number));
}

void RegisterWidget::setDisplayType(displayTypeN type) {
    // Given a display type "type", sets validators for the input.

    m_displayType = type;
    if (m_displayType == displayTypeN::Hex) {
        m_displayBase = 16;
        m_ui->value->setInputMask("hhhhhhhh");
        m_range = rangePair(0, (long long)4294967295);
        setText();
    } else if (m_displayType == displayTypeN::Binary) {
        // binary
        m_displayBase = 2;
        m_ui->value->setInputMask("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
        setText();
        m_range = rangePair(0, (long long)4294967295);
    } else if (m_displayType == displayTypeN::Decimal) {
        // Decimal
        m_displayBase = 10;
        m_ui->value->setInputMask("#0000000000");
        setText();
        m_range = rangePair(-(long long)2147483648, (long long)2147483647);
    } else if (m_displayType == displayTypeN::Unsigned) {
        // Unsigned
        m_displayBase = 10;
        m_ui->value->setInputMask("0000000000");
        setText();
        m_range = rangePair(0, (long long)4294967295);
    } else if (m_displayType == displayTypeN::ASCII) {
        // ASCII - valid ascii characters will not be able to fill the 32-bit
        // range
        m_ui->value->setInputMask("nnnn");
        setText();
        m_range = rangePair(0, (long long)4294967295);
    }
}

void RegisterWidget::enableInput(bool state) {
    // permanently called on reg[0], and on all registers when running
    // simulation,
    // to disable memory editing while running
    m_ui->value->setEnabled(state);
}
