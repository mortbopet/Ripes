#pragma once

#include <QWidget>
#include "iobase.h"

#include "ioledmatrix.h"
#include "ioswitches.h"

/** @brief IORegistry
 *
 * This is where all peripherals should be registerred to be made available in the UI.
 * The peripheral must be registerred three times:
 * - Add it to the IOType enum
 * - Add it to the IOTypeTitles map (Associate a name with the peripheral)
 * - Add it to the IOFactories map (Associate a constructor with the peripheral)
 */

namespace Ripes {

enum class IOType { LED_MATRIX, SWITCHES };

template <typename T>
IOBase* createIO(QWidget* parent, uint32_t startAddr) {
    static_assert(std::is_base_of<IOBase, T>::value);
    return new T(parent, startAddr);
}

using IOFactory = std::function<IOBase*(QWidget* parent, uint32_t)>;

const static std::map<IOType, QString> IOTypeTitles = {{IOType::LED_MATRIX, "LED Matrix"},
                                                       {IOType::SWITCHES, "Switches"}};
const static std::map<IOType, IOFactory> IOFactories = {{IOType::LED_MATRIX, createIO<IOLedMatrix>},
                                                        {IOType::SWITCHES, createIO<IOSwitches>}};

}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::IOType);
