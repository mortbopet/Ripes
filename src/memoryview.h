#ifndef MEMORYVIEW_H
#define MEMORYVIEW_H

#include <QTableView>
#include <QWheelEvent>

// Extension of QTableView - catches QWheelEvents and triggers the memory model
// to shift its
// central address

class MemoryView : public QTableView {
  Q_OBJECT

public:
  MemoryView(QWidget *parent = nullptr);

  void wheelEvent(QWheelEvent *event) override;

signals:
  void scrolled(bool dir);
};

#endif // MEMORYVIEW_H
