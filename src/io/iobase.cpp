#include "iobase.h"

namespace Ripes {

std::map<std::type_index, int> IOBase::s_peripheralCount;

IOBase::IOBase(QWidget* parent) : QWidget(parent) {
    registerPeripheral(this);

    connect(this, &IOBase::scheduleUpdate, this, QOverload<>::of(&QWidget::update));
}

bool IOBase::setParameter(unsigned ID, const QVariant& value) {
    const auto preSize = size();
    m_parameters.at(ID).value = value;
    parameterChanged(ID);
    const auto postSize = size();

    if (preSize != postSize) {
        emit sizeChanged();
    }

    return true;
}

}  // namespace Ripes
