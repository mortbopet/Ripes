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
    // Registers memory
    Graphics::Shape* registers = new Graphics::Shape(
      Graphics::ShapeType::Block, Graphics::Stage::ID, 10, 15);
    registers->addInput(QStringList() << "Read\nregister 1"
                                      << "Read\nregister 2"
                                      << "Write\nregister"
                                      << "Write\ndata");
    registers->addOutput(QStringList() << "Read\ndata 1"
                                       << "Read\ndata 2");
    registers->setName("Registers");

    // Data memory
    Graphics::Shape* data_mem = new Graphics::Shape(
      Graphics::ShapeType::Block, Graphics::Stage::MEM, 50, 5);
    data_mem->addInput(QStringList() << "Address"
                                     << "Write\ndata");
    data_mem->addOutput(QStringList() << "Read\ndata");
    data_mem->setName("Data\nmemory");

    // Instruction memory
    Graphics::Shape* instr_mem = new Graphics::Shape(Graphics::ShapeType::Block,
                                                     Graphics::Stage::IF, 0, 0);
    instr_mem->addInput(QStringList() << "Read\naddress");
    instr_mem->addOutput(QStringList() << "Instruction");
    instr_mem->setName("Instruction\nmemory");

    // PC
    Graphics::Shape* pc = new Graphics::Shape(Graphics::ShapeType::Block,
                                              Graphics::Stage::IF, 30, 3);
    pc->addInput("");
    pc->addOutput("");
    pc->setName("PC");

    // MUXes
    Graphics::Shape* mux1 =
      new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::IF, 20, 8);
    mux1->addInput(QStringList() << "0"
                                 << "1");
    mux1->addOutput("");
    mux1->setName("M\nu\nx");

    Graphics::Shape* mux2 =
      new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 20, 8);
    mux2->addInput(QStringList() << "0"
                                 << "1");
    mux2->addOutput("");
    mux2->setName("M\nu\nx");

    Graphics::Shape* mux3 =
      new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::WB, 20, 8);
    mux3->addInput(QStringList() << "0"
                                 << "1");
    mux3->addOutput("");
    mux3->setName("M\nu\nx");

    // ALUs
    Graphics::Shape* alu1 = new Graphics::Shape(Graphics::ShapeType::ALU,
                                                Graphics::Stage::ID, 70, 30);
    alu1->setName("Add");
    alu1->addOutput("Sum");

    Graphics::Shape* alu2 = new Graphics::Shape(Graphics::ShapeType::ALU,
                                                Graphics::Stage::EX, 70, 30);
    alu2->setName("Add");
    alu2->addOutput("Sum");

    Graphics::Shape* alu3 = new Graphics::Shape(Graphics::ShapeType::ALU,
                                                Graphics::Stage::EX, 70, 30);
    alu3->setName("Add");
    alu3->addOutput("Sum");

    // State registers
    Graphics::Shape* ifid = new Graphics::Shape(Graphics::ShapeType::Block,
                                                Graphics::Stage::ID, 150, 10);
    ifid->setName("IF/ID");
    ifid->addInput("");
    ifid->addOutput("");

    Graphics::Shape* idex = new Graphics::Shape(Graphics::ShapeType::Block,
                                                Graphics::Stage::EX, 150, 10);
    idex->setName("ID/EX");
    for (int i = 0; i < 6; i++) {
        idex->addInput("");
        idex->addOutput("");
    }

    // Add pipeline objects to graphics scene
    scene->addItem(mux1);
    scene->addItem(pc);
    scene->addItem(instr_mem);
    scene->addItem(alu1);
    /*
    scene->addItem(ifid);
    scene->addItem(registers);
    scene->addItem(data_mem);
    scene->addItem(mux2);
    scene->addItem(mux3);
    scene->addItem(alu2);
    scene->addItem(alu3);
    scene->addItem(idex);
*/
    // Create connections
    // Connections should be added in a left to right ordering!
    createConnection(mux1, 0, pc, 0);
    createConnection(pc, 0, instr_mem, 0);
    createConnection(pc, 0, alu1, 0);
    /*
    createConnection(instr_mem, 0, ifid, 0);
    createConnection(ifid, 0, registers, 0);
    createConnection(ifid, 0, registers, 1);
    createConnection(registers, 0, idex, 1);
    createConnection(registers, 1, idex, 2);
    createConnection(idex, 0, alu2, 0);
    createConnection(idex, 1, alu3, 0);
    createConnection(idex, 2, mux2, 0);
    */

    // Adjust positioning of items in scene
    adjustPositioning();
}

void PipelineWidget::createConnection(Graphics::Shape* source, int index1,
                                      Graphics::Shape* dest, int index2) {
    Graphics::Connection* connection =
      new Graphics::Connection(source, source->getOutputPoint(index1), dest,
                               dest->getInputPoint(index2));
    m_connections.append(connection);
    scene()->addItem(connection);
}

void PipelineWidget::adjustPositioning() {
    QList<Graphics::Shape*> movedShapes;
    const static int minimumLength = 40;  // Minimum connection length
    for (const auto& connection : m_connections) {
        auto shapes = connection->getShapes();
        if (!movedShapes.contains(shapes.first) &&
            !movedShapes.contains(shapes.second)) {
            // None of the shapes have been repositioned

            // Step 1 - set source and destination points to equal y coord
            auto pointPair = connection->getPoints();
            int newY = (pointPair.first.y() - pointPair.second.y()) / 2;

            // Step 2: set source and destination points to a sensible
            // x position. Middle point +- minimumLength/2
            // This also enforces that a source shape is positioned to the left
            // of a destination shape
            int dx = 0;
            int diff = pointPair.second.x() - pointPair.first.x();
            if (diff < minimumLength) {
                dx = (minimumLength - diff) / 2;
            }

            // Move shapes
            shapes.first->moveBy(-dx, -newY);
            shapes.second->moveBy(dx, newY);

            movedShapes.append(shapes.first);
            movedShapes.append(shapes.second);

            // Adjust connection to new positioning of its shapes
        } else if (movedShapes.contains(shapes.first) &&
                   !movedShapes.contains(shapes.second)) {
            // First shape has been moved, but not the second

            auto pointPair = connection->getPoints();
            int newY = (pointPair.first.y() - pointPair.second.y());
            int dx = 0;
            int diff = pointPair.second.x() - pointPair.first.x();
            if (diff < minimumLength) {
                dx = minimumLength - diff;
            }
            // Move shape
            shapes.second->moveBy(dx, newY);
            // shapes.second->calculatePoints();
            movedShapes.append(shapes.second);
        }
    }
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
