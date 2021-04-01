#include "iobase.h"

#include <condition_variable>

namespace Ripes {

std::map<unsigned, std::set<unsigned>> IOBase::s_peripheralIDs;

IOBase::IOBase(unsigned IOType, QWidget* parent) : QWidget(parent), m_type(IOType) {
    m_id = claimPeripheralId(m_type);
    connect(this, &IOBase::scheduleUpdate, this, QOverload<>::of(&QWidget::update));
}

QString cName(const QString& name) {
    QString cname = name;
    for (const auto& ch : {' ', '-'}) {
        cname.replace(ch, "_");
    }
    return cname.toUpper();
}

QString IOBase::name() const {
    return baseName() + " " + QString::number(m_id);
}

bool IOBase::setParameter(unsigned ID, const QVariant& value) {
    const auto preSize = byteSize();
    m_parameters.at(ID).value = value;
    parameterChanged(ID);
    const auto postSize = byteSize();
    if (preSize != postSize) {
        emit sizeChanged();
    }

    emit paramsChanged();
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
