#pragma once

#include <QColor>
#include <functional>
#include <memory>

namespace Ripes {
namespace Colors {

constexpr QColor BerkeleyBlue = {0x00, 0x32, 0x62};
constexpr QColor FoundersRock = {0x3B, 0x7E, 0xA1};
constexpr QColor CaliforniaGold = {0xFD, 0xB5, 0x15};
constexpr QColor Medalist = {0xC4, 0x82, 0x0E};
constexpr QColor FlatGreen = {0x4c, 0xde, 0x75};

// Returns a function which returns a lighter color on each instantiation
inline std::function<QColor()> incrementalColorGenerator(const QColor& start, unsigned steps) {
    auto color = std::make_shared<QColor>(start);
    return [color = color, steps] {
        const int decRatio = 100 + 80 / steps;
        QColor oldColor = *color;
        *color = color->lighter(decRatio);
        return oldColor;
    };
}

inline std::function<QColor()> incrementalRedGenerator(unsigned steps) {
    return incrementalColorGenerator(QColorConstants::Red.lighter(120), steps);
}

};  // namespace Colors
}  // namespace Ripes
