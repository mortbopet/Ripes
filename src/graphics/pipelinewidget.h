#ifndef PIPELINEWIDGET_H
#define PIPELINEWIDGET_H

#include <QGraphicsView>

class PipelineWidget : public QGraphicsView {
  Q_OBJECT
public:
  PipelineWidget(QWidget *parent = nullptr);
};

#endif // PIPELINEWIDGET_H
