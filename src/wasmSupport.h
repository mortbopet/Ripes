#pragma once

#include <QByteArray>
#include <QFileInfo>
#include <QWidget>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

namespace Ripes {

// Disable a widget if we are running in a wasm environment.
template <typename T>
#ifdef __EMSCRIPTEN__
inline void disableIfWasm(T *widget) {
  widget->setEnabled(false);
}
#else
inline void disableIfWasm(T *) {
}
#endif

// Disables a list of widgets if we are running in a wasm environment.
template <typename TList>
inline void disableIfWasm(const TList &widgets) {
  for (auto *widget : widgets)
    disableIfWasm(widget);
}

// The browser sandbox does not allow writing to the user's file system. If we
// are running in a wasm environment, offer 'data' as a browser download named
// after the file name of 'path'.
#ifdef __EMSCRIPTEN__
inline void downloadIfWasm(const QString &path, const QByteArray &data) {
  using emscripten::val;
  // Copy the data out of the (shared) wasm memory; Blobs cannot be created
  // from views of shared memory.
  val bytes = val::global("Uint8Array")
                  .new_(val(emscripten::typed_memory_view(
                      data.size(),
                      reinterpret_cast<const uint8_t *>(data.constData()))));
  val parts = val::array();
  parts.call<void>("push", bytes);
  val url = val::global("URL").call<val>("createObjectURL",
                                         val::global("Blob").new_(parts));
  val link =
      val::global("document").call<val>("createElement", std::string("a"));
  link.set("href", url);
  link.set("download", QFileInfo(path).fileName().toStdString());
  link.call<void>("click");
  val::global("URL").call<void>("revokeObjectURL", url);
}
#else
inline void downloadIfWasm(const QString &, const QByteArray &) {}
#endif

} // namespace Ripes
