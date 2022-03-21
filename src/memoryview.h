#pragma once

#include <QTableView>
#include <QWheelEvent>

namespace Ripes {

// Extension of QTableView - catches QWheelEvents and triggers the memory model
// to shift its
// central address

class MemoryView : public QTableView {
  Q_OBJECT

public:
  MemoryView(QWidget *parent = nullptr);

  void wheelEvent(QWheelEvent *event) override;

protected:
  void resizeEvent(QResizeEvent *event) override;

public slots:
  void setVisibleRows() {}

signals:
  void scrolled(bool dir);
  void resized();

private:
  int m_rowHeight = 0;
};
} // namespace Ripes
