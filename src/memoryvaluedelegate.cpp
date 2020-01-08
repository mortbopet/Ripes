#include "memoryvaluedelegate.h"

#include "radix.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QLineEdit>
#include <QMetaType>

MemoryValueDelegate::MemoryValueDelegate(QObject* parent) : QStyledItemDelegate(parent) {
    // The validator does not concern itself whether the value is actually writeable to the register. Any hex input is
    // accepted. When a register is forced to a value, the value is truncated to the bit width of the register.
    m_validator = new QRegExpValidator(this);
}

QWidget* MemoryValueDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const {
    QLineEdit* editor = new QLineEdit(parent);
    editor->setFont(QFont("monospace"));

    QPalette palette = QApplication::palette();
    palette.setBrush(QPalette::Text, Qt::blue);
    editor->setPalette(palette);
    editor->setValidator(m_validator);

    return editor;
}

void MemoryValueDelegate::setEditorData(QWidget* e, const QModelIndex& index) const {
    const auto radix = qvariant_cast<Radix>(index.data(Qt::UserRole));

    switch (radix) {
        case Radix::Binary: {
            m_validator->setRegExp(binRegex);
            break;
        }
        case Radix::Hex: {
            m_validator->setRegExp(hexRegex);
            break;
        }
        case Radix::Unsigned: {
            m_validator->setRegExp(unsignedRegex);
            break;
        }
        case Radix::Signed: {
            m_validator->setRegExp(signedRegex);
            break;
        }
    }

    QLineEdit* editor = dynamic_cast<QLineEdit*>(e);
    if (editor) {
        editor->setText(index.data().toString());
    }
}

void MemoryValueDelegate::setModelData(QWidget* e, QAbstractItemModel* model, const QModelIndex& index) const {
    QLineEdit* editor = dynamic_cast<QLineEdit*>(e);
    if (editor) {
        model->setData(index, editor->text(), Qt::EditRole);
    }
}
