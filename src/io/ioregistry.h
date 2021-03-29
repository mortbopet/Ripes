#pragma once

#include <QWidget>
#include "iobase.h"

#include "iodpad.h"
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

enum IOType { LED_MATRIX, SWITCHES, DPAD, NPERIPHERALS };

template <typename T>
IOBase* createIO(QWidget* parent) {
    static_assert(std::is_base_of<IOBase, T>::value);
    return new T(parent);
}

using IOFactory = std::function<IOBase*(QWidget* parent)>;

const static std::map<IOType, QString> IOTypeTitles = {{IOType::LED_MATRIX, "LED Matrix"},
                                                       {IOType::SWITCHES, "Switches"},
                                                       {IOType::DPAD, "D-Pad"}};
const static std::map<IOType, IOFactory> IOFactories = {{IOType::LED_MATRIX, createIO<IOLedMatrix>},
                                                        {IOType::SWITCHES, createIO<IOSwitches>},
                                                        {IOType::DPAD, createIO<IODPad>}};

}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::IOType);
