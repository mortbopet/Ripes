#include "pipelinewidget.h"
#include "shape.h"

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
      new Graphics::Shape(Graphics::ShapeType::Block, 15, 15);
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
      new Graphics::Shape(Graphics::ShapeType::Block, 50, 5);
    instr_mem->addInput(QStringList() << "Address");
    instr_mem->addOutput(QStringList() << "Instruction");
    instr_mem->setName("Instruction\nmemory");

    // PC
    Graphics::Shape* pc =
      new Graphics::Shape(Graphics::ShapeType::Block, 30, 3);
    pc->setName("PC");

    // Add pipeline objects to graphics scene
    scene->addItem(registers);
    scene->addItem(data_mem);
    scene->addItem(instr_mem);
    scene->addItem(pc);

    // Set item positions
    pc->setPos(-250, -100);
    instr_mem->setPos(-200, 0);
    registers->setPos(0, 0);
    data_mem->setPos(200, 0);
}
