#pragma once

#include <QEvent>
#include <QObject>

namespace Ripes {

class ScrollEventFilter : public QObject {
    Q_OBJECT

protected:
    bool eventFilter(QObject* obj, QEvent* event) override { return (event->type() == QEvent::Wheel); }
};

}  // namespace Ripes
