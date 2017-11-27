#include "pipelinewidget.h"
#include "backgrounditems.h"
#include "connection.h"
#include "defines.h"
#include "shape.h"

#include <QDebug>
#include <QRectF>

#include <QCoreApplication>
#include <QWheelEvent>

PipelineWidget::PipelineWidget(QWidget* parent) : QGraphicsView(parent) {
    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setCursor(Qt::CrossCursor);

    //  ------------ Create pipeline objects ---------------
    // Registers memory
    Graphics::Shape* registers = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::ID, 10, 15);
    registers->addInput(QStringList() << "Read\nregister 1"
                                      << "Read\nregister 2"
                                      << "Write\nregister"
                                      << "Write\ndata");
    registers->addOutput(QStringList() << "Read\ndata 1"
                                       << "Read\ndata 2");
    registers->setName("Registers");

    // Data memory
    Graphics::Shape* data_mem = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::MEM, 50, 5);
    data_mem->addInput(QStringList() << "Address"
                                     << "Write\ndata");
    data_mem->addOutput(QStringList() << "Read\ndata");
    data_mem->setName("Data\nmemory");

    // Instruction memory
    Graphics::Shape* instr_mem = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::IF, 60, 0);
    instr_mem->addInput(QStringList() << "Read\naddress");
    instr_mem->addOutput(QStringList() << "Instruction");
    instr_mem->setName("Instruction\nmemory");

    // PC
    Graphics::Shape* pc = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::IF, 30, 3);
    pc->addInput("");
    pc->addOutput("");
    pc->setName("PC");

    // MUXes
    Graphics::Shape* mux1 = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::IF, 20, 0);
    mux1->addInput(QStringList() << ""
                                 << "");
    mux1->addOutput("");
    mux1->setName("M\nu\nx");

    Graphics::Shape* mux2 = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 20, 0);
    mux2->addInput(QStringList() << ""
                                 << "");
    mux2->addOutput("");
    mux2->setName("M\nu\nx");

    Graphics::Shape* mux3 = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::WB, 20, 0);
    mux3->addInput(QStringList() << ""
                                 << "");
    mux3->addOutput("");
    mux3->setName("M\nu\nx");

    // ALUs
    Graphics::Shape* alu1 = new Graphics::Shape(Graphics::ShapeType::ALU, Graphics::Stage::ID, 70, 10);
    alu1->setName("+");
    alu1->addOutput("");

    Graphics::Shape* alu2 = new Graphics::Shape(Graphics::ShapeType::ALU, Graphics::Stage::EX, 70, 10);
    alu2->setName("+");
    alu2->addOutput("");

    Graphics::Shape* alu3 = new Graphics::Shape(Graphics::ShapeType::ALU, Graphics::Stage::EX, 70, 10);
    alu3->setName("ALU");
    alu3->addOutput("");

    // State registers
    ifid = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::ID, 0, 10);
    ifid->setName("IF/ID");
    ifid->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 2; i++) {
        ifid->addInput("");
        ifid->addOutput("");
    }

    idex = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::EX, 0, 10);
    idex->setName("ID/EX");
    idex->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 9; i++) {
        idex->addInput("");
        idex->addOutput("");
    }
    idex->addOutput("");

    exmem = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::EX, 0, 10);
    exmem->setName("EX/MEM");
    exmem->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 7; i++) {
        exmem->addInput("");
        exmem->addOutput("");
    }
    exmem->addOutput("");

    memwb = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::EX, 0, 10);
    memwb->setName("MEM/WB");
    memwb->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 4; i++) {
        memwb->addInput("");
        memwb->addOutput("");
    }
    memwb->addOutput("");

    // Static items
    Graphics::Shape* immgen = new Graphics::Shape(Graphics::ShapeType::Static, Graphics::Stage::EX, 20, 50);
    immgen->setName("Imm\ngen");
    immgen->addInput("");
    immgen->addOutput("");

    Graphics::Shape* sl1 = new Graphics::Shape(Graphics::ShapeType::Static, Graphics::Stage::EX, 40, 0);
    sl1->setName("Shift\nleft 1");
    sl1->addOutput("");
    sl1->drawBotPoint(true);

    // Add pipeline objects to graphics scene

    scene->addItem(mux1);
    scene->addItem(immgen);
    scene->addItem(pc);
    scene->addItem(instr_mem);
    scene->addItem(sl1);
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
    Graphics::Connection* connPtr;
    // IF
    createConnection(mux1, 0, pc, 0);
    createConnection(pc, 0, instr_mem, 0);
    createConnection(pc, 0, alu1, 0);
    connPtr = createConnection(alu1, 0, mux1, 0);
    connPtr->setFeedbackSettings(false, minConnectionLen - 20, minConnectionLen - 20);
    connPtr->setKinkBias(100);
    createConnection(instr_mem, 0, ifid, 1);
    createConnection(pc, 0, ifid, 0);
    // ID
    connPtr = createConnection(ifid, 0, registers, 0);
    connPtr->setKinkPoints(QList<int>() << 0 << 1);
    connPtr = createConnection(ifid, 0, registers, 1);
    connPtr->setKinkPoints(QList<int>() << 1);
    connPtr = createConnection(ifid, 0, registers, 2);
    createConnection(ifid, 0, immgen, 0);
    connPtr = createConnection(registers, 0, idex, 4);
    connPtr->setKinkBias(-10);
    connPtr = createConnection(registers, 1, idex, 5);
    connPtr->setKinkBias(10);
    createConnection(ifid, 0, idex, 3);
    createConnection(immgen, 0, idex, 6);
    // EX
    connPtr = createConnection(idex, 0, alu2, 0);
    connPtr->setKinkBias(-25);
    createConnection(idex, 4, alu3, 0);
    createConnection(idex, 6, mux2, 0);
    createConnection(mux2, 0, alu3, 1);
    createConnection(alu2, 0, exmem, 0);
    createConnection(alu3, 0, exmem, 1);
    createConnection(idex, sl1, idex->getOutputPoint(7), sl1->getBotPoint());
    createConnection(sl1, 0, alu2, 1);
    createConnection(idex, 2, exmem, 3);
    createConnection(idex, 6, mux2, 1);
    // MEM
    connPtr = createConnection(exmem, 0, mux1, 1);
    connPtr->setFeedbackSettings(false, minConnectionLen - 20, minConnectionLen);
    connPtr->setKinkBias(70);

    createConnection(exmem, 1, data_mem, 0);
    createConnection(exmem, 2, data_mem, 1);
    createConnection(data_mem, 0, memwb, 0);
    createConnection(exmem, 1, memwb, 1);
    // WB
    createConnection(memwb, 0, mux3, 0);
    createConnection(memwb, 1, mux3, 1);
    connPtr = createConnection(mux3, 0, registers, 3);
    connPtr->setFeedbackSettings(true, minConnectionLen - 20, minConnectionLen - 20);
    connPtr->setKinkBias(430);

    // ------- ITEM POSITIONING ------
    // Item positioning will mostly be done manually.
    // Most of it is based on positioning an "anchor" item, and calculating
    // other
    // items position relative to these

    // Position state registers
    ifid->moveBy(0, 0);
    idex->moveBy(spaceBetweenStateRegs * 1, 0);
    exmem->moveBy(spaceBetweenStateRegs * 2, 0);
    memwb->moveBy(spaceBetweenStateRegs * 3, 0);

    // position IF stage
    moveToIO(instr_mem, ifid, instr_mem->getOutputPoint(0), ifid->getInputPoint(1));
    pc->moveBy(instr_mem->sceneBoundingRect().left() - shapeMargin - pc->boundingRect().width(), 0);
    mux1->moveBy(pc->sceneBoundingRect().left() - shapeMargin - mux1->boundingRect().width(), 0);
    alu1->moveBy((instr_mem->sceneBoundingRect().left() / 3) - alu1->boundingRect().width(), -110);

    // position ID stage
    registers->moveBy(spaceBetweenStateRegs * 0.5, 0);
    moveToIO(immgen, idex, immgen->getOutputPoint(0), idex->getInputPoint(6));

    // position EX stage
    moveToIO(alu2, exmem, alu2->getOutputPoint(0), exmem->getInputPoint(2));
    moveToIO(alu3, exmem, alu3->getOutputPoint(0), exmem->getInputPoint(4));
    moveToIO(mux2, alu3, mux3->getOutputPoint(0), alu3->getInputPoint(1), 40);
    moveToIO(sl1, alu2, sl1->getOutputPoint(0), alu1->getInputPoint(1));
    mux2->moveBy(0, 1);

    // position MEM stage
    data_mem->moveBy(spaceBetweenStateRegs * 2.5, 0);

    // position WB stage
    moveToIO(mux3, memwb, mux3->getInputPoint(0), memwb->getOutputPoint(0), -minConnectionLen);

    // ----------- Create Etc. objects -----------
    Graphics::DashLine* dash1 = new Graphics::DashLine(ifid);
    Graphics::DashLine* dash2 = new Graphics::DashLine(idex);
    Graphics::DashLine* dash3 = new Graphics::DashLine(exmem);
    Graphics::DashLine* dash4 = new Graphics::DashLine(memwb);

    scene->addItem(dash1);
    scene->addItem(dash2);
    scene->addItem(dash3);
    scene->addItem(dash4);

    int             textPad  = 20;
    Graphics::Text* if_instr = new Graphics::Text(QPointF(
        -dash1->sceneBoundingRect().left() + dash1->sceneBoundingRect().width() / 2 + -spaceBetweenStateRegs / 2,
        dash1->sceneBoundingRect().top() + textPad));
    Graphics::Text* id_instr = new Graphics::Text(
        QPointF(dash1->sceneBoundingRect().left() + dash1->sceneBoundingRect().width() / 2 + spaceBetweenStateRegs / 2,
                dash1->sceneBoundingRect().top() + textPad));
    Graphics::Text* ex_instr = new Graphics::Text(
        QPointF(dash2->sceneBoundingRect().left() + dash2->sceneBoundingRect().width() / 2 + spaceBetweenStateRegs / 2,
                dash2->sceneBoundingRect().top() + textPad));
    Graphics::Text* mem_instr = new Graphics::Text(
        QPointF(dash3->sceneBoundingRect().left() + dash3->sceneBoundingRect().width() / 2 + spaceBetweenStateRegs / 2,
                dash3->sceneBoundingRect().top() + textPad));
    Graphics::Text* wb_instr = new Graphics::Text(QPointF(dash4->sceneBoundingRect().left() + spaceBetweenStateRegs / 3,
                                                          dash4->sceneBoundingRect().top() + textPad));

    if_instr->setTextPtr(new QString("and x12, x2, x5"));
    id_instr->setTextPtr(new QString("beq x1, x3, 16"));
    ex_instr->setTextPtr(new QString("sub x10, x4, x8"));
    mem_instr->setTextPtr(new QString("nop"));
    wb_instr->setTextPtr(new QString("rd x10 24 x2"));

    scene->addItem(if_instr);
    scene->addItem(id_instr);
    scene->addItem(ex_instr);
    scene->addItem(mem_instr);
    scene->addItem(wb_instr);
}

void PipelineWidget::moveToIO(Graphics::Shape* source, Graphics::Shape* dest, QPointF* sourcePoint, QPointF* destPoint,
                              int connectionLength) {
    // Moves the source shape such that its sourcePoint is aligned with the
    // destination point, seperated by connectionLength
    QPointF sceneSourcePoint = source->mapToScene(*sourcePoint);
    QPointF sceneDestPoint   = dest->mapToScene(*destPoint);

    int newY = sceneSourcePoint.y() - sceneDestPoint.y();
    int newX = sceneSourcePoint.x() - sceneDestPoint.x() + connectionLength;

    // Move shapes
    source->moveBy(-newX, -newY);
}

Graphics::Connection* PipelineWidget::createConnection(Graphics::Shape* source, int index1, Graphics::Shape* dest,
                                                       int index2) {
    // for testing
    static uint32_t value = 1;
    value <<= 1;
    //
    Graphics::Connection* connection =
        new Graphics::Connection(source, source->getOutputPoint(index1), dest, dest->getInputPoint(index2));
    connect(this, &PipelineWidget::displayAllValuesSig, connection, &Graphics::Connection::showValue);
    connection->setValue(value);
    m_connections.append(connection);
    source->addConnection(connection);
    dest->addConnection(connection);
    scene()->addItem(connection);
    connection->addLabelToScene();
    return connection;
}

Graphics::Connection* PipelineWidget::createConnection(Graphics::Shape* source, Graphics::Shape* dest,
                                                       QPointF* sourcePoint, QPointF* destPoint) {
    Graphics::Connection* connection = new Graphics::Connection(source, sourcePoint, dest, destPoint);
    m_connections.append(connection);
    source->addConnection(connection);
    dest->addConnection(connection);
    scene()->addItem(connection);
    return connection;
}

QList<QGraphicsItem*> PipelineWidget::filterAllowedItems(Graphics::Shape* shape, QList<QGraphicsItem*> items) {
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

void PipelineWidget::wheelEvent(QWheelEvent* event) {
    scaleView(pow((double)2, -event->delta() / 350.0));
}

void PipelineWidget::scaleView(qreal scaleFactor) {
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}

void PipelineWidget::displayAllValues(bool state) {
    emit displayAllValuesSig(state);
    scene()->update();
}
