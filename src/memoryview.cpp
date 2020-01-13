#include "memoryview.h"

#include "memorymodel.h"

#include <QHeaderView>

namespace Ripes {

MemoryView::MemoryView(QWidget* parent) : QTableView(parent) {}

void MemoryView::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        emit scrolled(true);
    } else {
        emit scrolled(false);
    }
}

void MemoryView::resizeEvent(QResizeEvent* event) {
    QTableView::resizeEvent(event);
    emit resized();
}
}  // namespace Ripes
