#include "pipelinewidget.h"
#include "shape.h"

PipelineWidget::PipelineWidget(QWidget *parent) : QGraphicsView(parent) {
  QGraphicsScene *scene = new QGraphicsScene(this);
  scene->setSceneRect(rect());
  scene->setItemIndexMethod(QGraphicsScene::NoIndex);
  setScene(scene);
  setCacheMode(CacheBackground);
  setRenderHint(QPainter::Antialiasing);
  scale(qreal(0.8), qreal(0.8));

  // Create pipeline objects
  Graphics::Shape *instr_mem = new Graphics::Shape();
  Graphics::Shape *registers = new Graphics::Shape();
  Graphics::Shape *data_mem = new Graphics::Shape();

  // Add pipeline objects to graphics scene
  scene->addItem(instr_mem);
  scene->addItem(registers);
  scene->addItem(data_mem);

  // Set item positions
  instr_mem->setPos(50, 0);
}
