#include "iomanager.h"

namespace Ripes {

IOManager::IOManager() {}

IOBase* IOManager::createPeripheral(IOType type) {
    auto* peripheral = IOFactories.at(type)(nullptr);

    // Assign base address to peripheral

    return peripheral;
}

}  // namespace Ripes
