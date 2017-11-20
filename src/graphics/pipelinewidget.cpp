#include "pipelinewidget.h"
#include "connection.h"
#include "shape.h"

#include <QDebug>
#include <QRectF>

#include <QCoreApplication>
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
                                                Graphics::Stage::ID, 350, 10);
    ifid->setName("IF/ID");
    for (int i = 0; i < 2; i++) {
        ifid->addInput("");
        ifid->addOutput("");
    }

    Graphics::Shape* idex = new Graphics::Shape(Graphics::ShapeType::Block,
                                                Graphics::Stage::EX, 350, 10);
    idex->setName("ID/EX");
    for (int i = 0; i < 4; i++) {
        idex->addInput("");
        idex->addOutput("");
    }

    Graphics::Shape* exmem = new Graphics::Shape(Graphics::ShapeType::Block,
                                                 Graphics::Stage::EX, 350, 10);
    exmem->setName("EX/MEM");
    for (int i = 0; i < 4; i++) {
        exmem->addInput("");
        exmem->addOutput("");
    }

    Graphics::Shape* memwb = new Graphics::Shape(Graphics::ShapeType::Block,
                                                 Graphics::Stage::EX, 350, 10);
    memwb->setName("MEM/WB");
    for (int i = 0; i < 2; i++) {
        memwb->addInput("");
        memwb->addOutput("");
    }

    // add a point at 0,0 for testing purposes
    scene->addEllipse(0, 0, 5, 5, QPen(Qt::red));

    // Add pipeline objects to graphics scene
    scene->addItem(mux1);
    scene->addItem(pc);
    scene->addItem(instr_mem);
    scene->addItem(alu1);
    scene->addItem(ifid);
    scene->addItem(registers);
    scene->addItem(data_mem);
    scene->addItem(mux2);
    scene->addItem(mux3);
    scene->addItem(alu2);
    scene->addItem(alu3);
    scene->addItem(idex);
    scene->addItem(exmem);
    scene->addItem(memwb);

    //  ----------- Create connections ----------------------
    // IF
    createConnection(mux1, 0, pc, 0);
    createConnection(pc, 0, instr_mem, 0);
    createConnection(pc, 0, alu1, 0);
    createConnection(alu1, 0, mux1, 0);
    createConnection(instr_mem, 0, ifid, 1);
    createConnection(pc, 0, ifid, 0);
    // ID
    createConnection(ifid, 0, registers, 0);
    createConnection(ifid, 0, registers, 1);
    createConnection(ifid, 0, registers, 2);
    createConnection(registers, 0, idex, 1);
    createConnection(registers, 1, idex, 2);
    createConnection(ifid, 0, idex, 0);
    // EX
    createConnection(idex, 0, alu2, 0);
    createConnection(idex, 1, alu3, 0);
    createConnection(idex, 2, mux2, 0);
    createConnection(mux2, 0, alu3, 1);
    createConnection(alu2, 0, exmem, 0);
    createConnection(alu3, 0, exmem, 1);
    createConnection(idex, 2, exmem, 3);
    createConnection(idex, 3, alu2, 1);
    // MEM
    createConnection(exmem, 0, mux1, 1);
    createConnection(exmem, 1, data_mem, 0);
    createConnection(exmem, 2, data_mem, 1);
    createConnection(data_mem, 0, memwb, 0);
    createConnection(exmem, 1, memwb, 1);
    // WB
    createConnection(memwb, 0, mux3, 0);
    createConnection(memwb, 1, mux3, 1);
    createConnection(mux3, 0, registers, 3);

    // Adjust positioning of items in scene
    adjustPositioning();
}

void PipelineWidget::createConnection(Graphics::Shape* source, int index1,
                                      Graphics::Shape* dest, int index2) {
    Graphics::Connection* connection =
      new Graphics::Connection(source, source->getOutputPoint(index1), dest,
                               dest->getInputPoint(index2));
    m_connections.append(connection);
    source->addConnection(connection);
    dest->addConnection(connection);
    scene()->addItem(connection);
}

QList<QGraphicsItem*> PipelineWidget::filterAllowedItems(
  Graphics::Shape* shape, QList<QGraphicsItem*> items) {
    // Removes the shape itself and all connections from items
    auto returnList = items;
    for (const auto& item : items) {
        if (item->type() == Graphics::Connection::connectionType()) {
            returnList.removeAll(item);

        } else if (item->type() == Graphics::Shape::connectionType()) {
            if (shape == item) {
                // Remove item if it intersects with itself (this happens when
                // we just check what is intersecting with a rectangle in the
                // scene
                // -ofcourse the item itself will intersect that rectangle)
                returnList.removeAll(item);
            }
        }
    }
    return returnList;
}

void PipelineWidget::adjustPositioning() {
    QList<Graphics::Shape*> movedShapes;
    for (const auto& connection : m_connections) {
        auto shapes = connection->getShapes();
        if (!movedShapes.contains(shapes.first) &&
            !movedShapes.contains(shapes.second)) {
            // None of the shapes have been repositioned

            // Step 1 - set source and destination points to equal y coord
            auto pointPair = connection->getPoints();
            int newY = (pointPair.first.y() - pointPair.second.y()) / 2;

            // Step 2: set source and destination points to a sensible
            // x position. Middle point +- minConnectionLen/2
            // This also enforces that a source shape is positioned to the left
            // of a destination shape
            int dx = 0;
            int diff = pointPair.second.x() - pointPair.first.x();
            if (diff < minConnectionLen) {
                dx = (minConnectionLen - diff) / 2;
            }

            // Move shapes
            shapes.first->moveBy(-dx, -newY);
            shapes.second->moveBy(dx, newY);
            QCoreApplication::processEvents();

            movedShapes.append(shapes.first);
            movedShapes.append(shapes.second);

            // Adjust connection to new positioning of its shapes
        } else if (movedShapes.contains(shapes.first) &&
                   !movedShapes.contains(shapes.second)) {
            // First shape has been moved, but not the second
            auto pointPair = connection->getPoints();
            int newY = (pointPair.first.y() - pointPair.second.y());
            int dx = 0;
            int diff =
              pointPair.second.x() - shapes.first->sceneBoundingRect().right();
            if (diff < minConnectionLen) {
                dx = minConnectionLen - diff;
            }
            // Move shape to the new position. Afterwards, process all pending
            // events
            // in the event loop - this will indirectly update the scene. We
            // need to
            // verify that the scene has been updated before checking
            // intersecting
            // items

            shapes.second->moveBy(dx, newY);
            QCoreApplication::processEvents();
            auto newRect = shapes.second->sceneBoundingRect();
            // Check for intersections with other shapes
            auto intersectingItems =
              items(newRect.toRect(), Qt::IntersectsItemBoundingRect);
            intersectingItems =
              filterAllowedItems(shapes.second, intersectingItems);

            if (intersectingItems.size() == 0) {
                // Accept new position
                movedShapes.append(shapes.second);
            } else {
                // Intersecting items detected. Move second shape in air up or
                // down, until the shapeMargin requirement is met
                QRect rectBelow = newRect.toRect();
                QRect rectAbove = newRect.toRect();
                auto intersectingRect =
                  intersectingItems.first()->boundingRect();
                // Translate the shapes rect to be below and above the
                // intersecting item
                rectBelow.translate(
                  0, intersectingRect.bottom() - rectBelow.top() + shapeMargin);
                rectAbove.translate(
                  0, intersectingRect.top() - rectAbove.bottom() - shapeMargin);

                // We prefer a new position that is closest to y = 0; order
                // the top and bottom rectangles with respect to this
                QPair<QRect, QRect> rectOrdered =
                  abs(rectBelow.top()) > abs(rectAbove.bottom())
                    ? QPair<QRect, QRect>(rectBelow, rectAbove)
                    : QPair<QRect, QRect>(rectAbove, rectBelow);

                // Check for intersection
                intersectingItems =
                  items(rectOrdered.first, Qt::IntersectsItemBoundingRect);
                intersectingItems =
                  filterAllowedItems(nullptr, intersectingItems);

                if (intersectingItems.size() == 0) {
                    int a = intersectingRect.bottom();
                    a += -rectOrdered.first.bottom();
                    a += shapeMargin;
                    shapes.second->moveBy(0, a);
                }
                movedShapes.append(shapes.second);
            }
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
