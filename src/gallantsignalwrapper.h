#pragma once

#include <QMetaObject>
#include <functional>
#include "Signals/Signal.h"

namespace Ripes {
/**
 * class CrossThreadSignalWrapper
 * This class acts as a wrapper for Gallant signals. @p fun will be executed within the event loop context of @p obj,
 * upon signal @p sig being triggered. This serves the following purposes:
 * - Gallant signals only accepts raw pointers as callback functions; we'd like to be able to pass a lambda function.
 * - Gallant signals are not thread safe. The wrapper will ensure that signals go through the signal/slot system of Qt,
 * which checks for, and handles, cross-thread signals.
 */
struct GallantSignalWrapperBase {
    virtual ~GallantSignalWrapperBase(){};
};

template <typename F, typename D, typename T>
class GallantSignalWrapper : public GallantSignalWrapperBase {
public:
    GallantSignalWrapper(T* obj, F&& fun, D& sig, Qt::ConnectionType type = Qt::AutoConnection) {
        wrapper = [=] { QMetaObject::invokeMethod(obj, fun, type); };
        sig.Connect(this, &GallantSignalWrapper::ftunnel);
    }

private:
    void ftunnel() { wrapper(); }
    std::function<void()> wrapper;
};
}  // namespace Ripes
