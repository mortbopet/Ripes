#pragma once

#include <QRegExpValidator>
#include <QStyledItemDelegate>

class MemoryValueDelegate : public QStyledItemDelegate {
public:
    MemoryValueDelegate(QObject* parent);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private:
    QRegExpValidator* m_validator;
};
