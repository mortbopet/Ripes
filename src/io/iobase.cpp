#include "iobase.h"

#include <condition_variable>

namespace Ripes {

std::map<std::type_index, int> IOBase::s_peripheralCount;

IOBase::IOBase(QWidget* parent) : QWidget(parent) {
    incrementPeriphCounter(this);
    connect(this, &IOBase::scheduleUpdate, this, QOverload<>::of(&QWidget::update));
}

bool IOBase::setParameter(unsigned ID, const QVariant& value) {
    const auto preSize = size();
    m_parameters.at(ID).value = value;
    parameterChanged(ID);
    const auto postSize = size();

    if (preSize != postSize) {
        emit sizeChanged();
    }

    return true;
}

void IOBase::unregister() {
    std::atomic<bool> sync;
    std::condition_variable cv;
    std::mutex cv_m;
    std::unique_lock<std::mutex> lock(cv_m);

    // Signal to IO Manager that we're about to delete this peripheral, allowing it to unregister this.
    emit aboutToDelete(sync);
    cv.wait(lock, [&sync] { return sync == 1; });
    m_didUnregister = true;

    // Continue destruction process...
}

}  // namespace Ripes
