#pragma once

#include <QStringList>

namespace Ripes {

inline int getImmediate(QString string, bool& canConvert) {
    string = string.toUpper();
    canConvert = false;
    int immediate = string.toInt(&canConvert, 10);
    int sign = 1;
    if (!canConvert) {
        // Could not convert directly to integer - try hex or bin. Here, extra care is taken to account for a
        // potential sign, and include this is the range validation
        if (string[0] == '-' || string[0] == '+') {
            sign = string[0] == '-' ? -1 : 1;
            string.remove(0, 1);
        }
        if (string.startsWith(QLatin1String("0X"))) {
            immediate = string.remove("0X").toUInt(&canConvert, 16);
        } else if (string.startsWith(QLatin1String("0B"))) {
            immediate = string.remove("0B").toUInt(&canConvert, 2);
        } else {
            canConvert = false;
        }
    }
    return sign * immediate;
}

}  // namespace Ripes
