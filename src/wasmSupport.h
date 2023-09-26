#pragma once

#include <QWidget>

namespace Ripes {

// Disable a widget if we are running in a wasm environment.
template <typename T>
inline void disableIfWasm(T *widget) {
#ifdef __EMSCRIPTEN__
  widget->setEnabled(false);
#endif
}

// Disables a list of widgets if we are running in a wasm environment.
template <typename TList>
inline void disableIfWasm(TList widgets) {
  for (auto *widget : widgets)
    disableIfWasm(widget);
}

} // namespace Ripes