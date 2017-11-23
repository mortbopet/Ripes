#include "memoryview.h"

#include "memorymodel.h"

#include <QHeaderView>

MemoryView::MemoryView(QWidget* parent) : QTableView(parent) {}

void MemoryView::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        emit scrolled(true);
    } else {
        emit scrolled(false);
    }
}

void MemoryView::setVisibleRows() {
    // Detect and set changes in rowheight
    if (rowHeight(1) != 0) {
        m_rowHeight = rowHeight(1);
    }

    // Set visible rows from model, based on tableView size and rowheights
    int modelRows = dynamic_cast<MemoryModel*>(model())->getModelRows();

    // Calculate number of visible rows
    int visibleRows = (height() - horizontalHeader()->height() - 10) / m_rowHeight;
    if ((visibleRows % 2) == 0)
        visibleRows--;

    // Hide the rows above and below the requested range
    const int removeCnt = (modelRows - visibleRows) / 2;
    for (int i = 0; i < removeCnt; i++) {
        setRowHidden(i, true);
    }
    for (int i = removeCnt; i < (visibleRows + removeCnt); i++) {
        setRowHidden(i, false);
    }
    for (int i = visibleRows + removeCnt; i < (visibleRows + 2 * removeCnt); i++) {
        setRowHidden(i, true);
    }
}

void MemoryView::resizeEvent(QResizeEvent* event) {
    QTableView::resizeEvent(event);
    setVisibleRows();
}
