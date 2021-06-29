#pragma once

#include <set>

#include <QAbstractTableModel>
#include <QColor>

#include "processors/interface/ripesprocessor.h"
#include "radix.h"

namespace Ripes {

class RegisterModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Name, Alias, Value, NColumns };
    RegisterModel(RegisterFileType rft, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setRadix(Radix r);
    Radix getRadix() const { return m_radix; }

public slots:
    void processorWasClocked();

signals:
    /**
     * @brief registerChanged
     * Emitted whenever the value of register @param i changes
     */
    void registerChanged(unsigned i);

private:
    std::vector<VInt> gatherRegisterValues();

    QVariant nameData(unsigned idx) const;
    QVariant aliasData(unsigned idx) const;
    QVariant valueData(unsigned idx) const;
    QVariant tooltipData(unsigned idx) const;

    Radix m_radix = Radix::Hex;
    unsigned m_regBytes;
    RegisterFileType m_rft;

    int m_mostRecentlyModifiedReg = -1;
    std::vector<VInt> m_regValues;
};
}  // namespace Ripes
