#include "iobase.h"

namespace Ripes {

std::map<std::type_index, int> IOBase::s_peripheralCount;

bool IOBase::setParameter(unsigned ID, const QVariant& value) {
    m_parameters.at(ID).value = value;
    parameterChanged(ID);
    return true;
}

}  // namespace Ripes
