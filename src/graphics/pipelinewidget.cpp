#include "pipelinewidget.h"
#include "shape.h"

PipelineWidget::PipelineWidget(QWidget* parent) : QGraphicsView(parent) {
    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->setSceneRect(-200, -200, 400, 400);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    setCacheMode(CacheBackground);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);

    // Create pipeline objects
    Graphics::Shape* instr_mem = new Graphics::Shape();
    instr_mem->addInput(QStringList() << "Read\nregister 1"
                                      << "Read\nregister 2"
                                      << "Write\nregister"
                                      << "Write\ndata");
    instr_mem->addOutput(QStringList() << "Read\ndata 1"
                                       << "Read\ndata 2");
    instr_mem->setName("Registers");

    // Add pipeline objects to graphics scene
    scene->addItem(instr_mem);

    // Set item positions
    instr_mem->setPos(0, 0);
}
