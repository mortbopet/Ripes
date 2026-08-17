#pragma once

#include <QColor>
#include <QGuiApplication>
#include <QPalette>
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
inline std::function<QColor()> incrementalColorGenerator(const QColor &start,
                                                         unsigned steps) {
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

}; // namespace Colors

/**
 * @brief The SyntaxColorScheme struct
 * Foreground colors used for source-code syntax highlighting.
 */
struct SyntaxColorScheme {
  QColor keyword;
  QColor type;
  QColor instruction;
  QColor function;
  QColor registerName;
  QColor immediate;
  QColor string;
  QColor label;
  QColor comment;
  QColor preprocessor;
};

/**
 * @brief currentSyntaxColors
 * Returns a syntax color scheme appropriate for the currently active
 * application color scheme (light or dark). Light mode preserves the original
 * Ripes colors; dark mode uses lighter, higher-contrast variants.
 */
inline SyntaxColorScheme currentSyntaxColors() {
  const bool dark =
      QGuiApplication::palette().color(QPalette::Base).lightness() < 128;
  if (dark) {
    return SyntaxColorScheme{
        /*keyword*/ QColor(0xC6, 0x78, 0xDD),
        /*type*/ QColor(0x56, 0xB6, 0xC2),
        /*instruction*/ QColor(0x61, 0xAF, 0xEF),
        /*function*/ QColor(0x61, 0xAF, 0xEF),
        /*registerName*/ QColor(0xE0, 0x6C, 0x75),
        /*immediate*/ QColor(0xD1, 0x9A, 0x66),
        /*string*/ QColor(0x98, 0xC3, 0x79),
        /*label*/ QColor(0xE5, 0xC0, 0x7B),
        /*comment*/ QColor(0x7F, 0x84, 0x8E),
        /*preprocessor*/ QColor(0xC6, 0x78, 0xDD),
    };
  }
  return SyntaxColorScheme{
      /*keyword*/ QColor(Qt::darkBlue),
      /*type*/ QColor(Qt::darkBlue),
      /*instruction*/ Colors::BerkeleyBlue,
      /*function*/ Colors::BerkeleyBlue,
      /*registerName*/ QColor(0x80, 0x00, 0x00),
      /*immediate*/ QColor(QColorConstants::DarkGreen),
      /*string*/ QColor(0x80, 0x00, 0x00),
      /*label*/ Colors::Medalist,
      /*comment*/ Colors::Medalist,
      /*preprocessor*/ QColor(QColorConstants::DarkMagenta),
  };
}

} // namespace Ripes
