#include "pipelinewidget.h"
#include "connection.h"
#include "shape.h"

#include <QWheelEvent>

PipelineWidget::PipelineWidget(QWidget* parent) : QGraphicsView(parent) {
  QGraphicsScene* scene = new QGraphicsScene(this);
  scene->setItemIndexMethod(QGraphicsScene::NoIndex);
  setScene(scene);
  setCacheMode(CacheBackground);
  setRenderHint(QPainter::Antialiasing);
  setRenderHint(QPainter::TextAntialiasing);
  setDragMode(QGraphicsView::ScrollHandDrag);

  //  ------------ Create pipeline objects ---------------
  // Instruction memory
  Graphics::Shape* registers =
      new Graphics::Shape(Graphics::ShapeType::Block, 10, 15);
  registers->addInput(QStringList() << "Read\nregister 1"
                                    << "Read\nregister 2"
                                    << "Write\nregister"
                                    << "Write\ndata");
  registers->addOutput(QStringList() << "Read\ndata 1"
                                     << "Read\ndata 2");
  registers->setName("Registers");

  // Data memory
  Graphics::Shape* data_mem =
      new Graphics::Shape(Graphics::ShapeType::Block, 50, 5);
  data_mem->addInput(QStringList() << "Address"
                                   << "Write\ndata");
  data_mem->addOutput(QStringList() << "Read\ndata");
  data_mem->setName("Data\nmemory");

  // Instruction memory
  Graphics::Shape* instr_mem =
      new Graphics::Shape(Graphics::ShapeType::Block, 30, 0);
  instr_mem->addInput(QStringList() << "Read\naddress");
  instr_mem->addOutput(QStringList() << "Instruction");
  instr_mem->setName("Instruction\nmemory");

  // PC
  Graphics::Shape* pc = new Graphics::Shape(Graphics::ShapeType::Block, 30, 3);
  pc->addInput("");
  pc->addOutput("");
  pc->setName("PC");

  // MUX
  Graphics::Shape* mux1 = new Graphics::Shape(Graphics::ShapeType::MUX, 20, 8);
  mux1->addInput(QStringList() << "0"
                               << "1"
                               << "2");
  mux1->addOutput("C");
  mux1->setName("M\nu\nx");

  // ALU
  Graphics::Shape* ALU1 = new Graphics::Shape(Graphics::ShapeType::ALU, 70, 30);
  ALU1->setName("Add");
  ALU1->addOutput("Sum");

  // Add pipeline objects to graphics scene
  scene->addItem(registers);
  scene->addItem(data_mem);
  scene->addItem(instr_mem);
  scene->addItem(pc);
  scene->addItem(mux1);
  scene->addItem(ALU1);

  // Set item positions
  pc->setPos(-250, -100);
  instr_mem->setPos(-300, 0);
  registers->setPos(0, 0);
  data_mem->setPos(200, 0);
  mux1->setPos(0, -200);
  ALU1->setPos(100, 200);

  // Create connections
  scene->addItem(new Graphics::Connection(
      registers, registers->getOutputPoint(1), mux1, mux1->getInputPoint(0)));
}

void PipelineWidget::wheelEvent(QWheelEvent* event) {
  scaleView(pow((double)2, -event->delta() / 350.0));
}

void PipelineWidget::scaleView(qreal scaleFactor) {
  qreal factor = transform()
                     .scale(scaleFactor, scaleFactor)
                     .mapRect(QRectF(0, 0, 1, 1))
                     .width();
  if (factor < 0.07 || factor > 100) return;

  scale(scaleFactor, scaleFactor);
}
