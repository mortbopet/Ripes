#include "memoryview.h"

MemoryView::MemoryView(QWidget *parent) : QTableView(parent) {}

void MemoryView::wheelEvent(QWheelEvent *event) {
  if (event->angleDelta().y() > 0) {
    emit scrolled(true);
  } else {
    emit scrolled(false);
  }
}
