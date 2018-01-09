#include "pipelinewidget.h"
#include "backgrounditems.h"
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
    instr_mem->addInput(QStringList() << "");
    instr_mem->addOutput(QStringList() << "");
    instr_mem->setName("Instruction\nmemory");

    // PC
    Graphics::Shape* pc = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::IF, 30, 3);
    pc->addInput("");
    pc->addOutput("");
    pc->addBotPoint("");
    pc->setName("PC");

    // MUXes
    Graphics::Shape* mux_PCSrc = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::IF, 0, 0);
    mux_PCSrc->addInput(QStringList() << ""
                                      << ""
                                      << "");
    mux_PCSrc->addOutput("");
    mux_PCSrc->addBotPoint("");
    mux_PCSrc->setName("M\nu\nx");

    Graphics::Shape* mux_forwardA_EX = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 0, 0);
    mux_forwardA_EX->addInput(QStringList() << ""
                                            << ""
                                            << "");
    mux_forwardA_EX->addOutput("");
    mux_forwardA_EX->setName("M\nu\nx");

    Graphics::Shape* mux_forwardB_EX = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 0, 0);
    mux_forwardB_EX->addInput(QStringList() << ""
                                            << ""
                                            << "");
    mux_forwardB_EX->addOutput("");
    mux_forwardB_EX->setName("M\nu\nx");

    Graphics::Shape* mux_ALUSrc1 = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 0, 0);
    mux_ALUSrc1->addInput(QStringList() << ""
                                        << "");
    mux_ALUSrc1->addOutput("");
    mux_ALUSrc1->setName("M\nu\nx");

    Graphics::Shape* mux_ALUSrc2 = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 0, 0);
    mux_ALUSrc2->addInput(QStringList() << ""
                                        << "");
    mux_ALUSrc2->addOutput("");
    mux_ALUSrc2->setName("M\nu\nx");

    Graphics::Shape* mux_alures_PC4_MEM = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 0, 0);
    mux_alures_PC4_MEM->addInput(QStringList() << ""
                                               << "");
    mux_alures_PC4_MEM->addOutput("");
    mux_alures_PC4_MEM->setName("M\nu\nx");

    Graphics::Shape* mux_memToReg = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::WB, 0, 0);
    mux_memToReg->addInput(QStringList() << ""
                                         << ""
                                         << "");
    mux_memToReg->addBotPoint("");
    mux_memToReg->addOutput("");
    mux_memToReg->setName("M\nu\nx");

    // ALUs
    Graphics::Shape* alu_pc4 = new Graphics::Shape(Graphics::ShapeType::ALU, Graphics::Stage::ID, 70, 10);
    alu_pc4->setName("+");
    alu_pc4->addOutput("");

    Graphics::Shape* alu_pc_target = new Graphics::Shape(Graphics::ShapeType::ALU, Graphics::Stage::EX, 70, 10);
    alu_pc_target->setName("+");
    alu_pc_target->addOutput("");

    Graphics::Shape* alu_mainALU = new Graphics::Shape(Graphics::ShapeType::ALU, Graphics::Stage::EX, 135, 10);
    alu_mainALU->setName("ALU");
    alu_mainALU->addOutput("");

    // State registers
    ifid = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::ID, 0, 10);
    ifid->setName("IF/ID");
    ifid->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 5; i++) {
        ifid->addInput("");
        ifid->addOutput("");
    }
    ifid->setHiddenOutputs(std::set<int>{3, 4});

    idex = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::EX, 0, 10);
    idex->setName("ID/EX");
    idex->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 12; i++) {
        idex->addInput("");
        idex->addOutput("");
    }
    idex->setHiddenOutputs(std::set<int>{2, 11});

    exmem = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::EX, 0, 10);
    exmem->setName("EX/MEM");
    exmem->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 6; i++) {
        exmem->addInput("");
        exmem->addOutput("");
    }
    exmem->setHiddenOutputs(std::set<int>{1});

    memwb = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::EX, 0, 10);
    memwb->setName("MEM/WB");
    memwb->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 5; i++) {
        memwb->addInput("");
        memwb->addOutput("");
    }

    // Static items
    Graphics::Shape* immgen = new Graphics::Shape(Graphics::ShapeType::Static, Graphics::Stage::EX, 20, 0);
    immgen->setName("Imm\ngen");
    immgen->addInput("");
    immgen->addOutput("");

    // Add pipeline objects to graphics scene

    scene->addItem(mux_PCSrc);
    scene->addItem(immgen);
    scene->addItem(pc);
    scene->addItem(instr_mem);
    scene->addItem(alu_pc4);
    scene->addItem(ifid);
    scene->addItem(registers);
    scene->addItem(data_mem);
    scene->addItem(mux_forwardA_EX);
    scene->addItem(mux_forwardB_EX);
    scene->addItem(mux_ALUSrc1);
    scene->addItem(mux_ALUSrc2);
    scene->addItem(mux_alures_PC4_MEM);
    scene->addItem(mux_memToReg);
    scene->addItem(alu_pc_target);
    scene->addItem(alu_mainALU);
    scene->addItem(idex);
    scene->addItem(exmem);
    scene->addItem(memwb);

    //  ----------- Create connections ----------------------
    Graphics::Connection* connPtr;
    // IF
    createConnection(mux_PCSrc, 0, pc, 0);
    createConnection(pc, 0,
                     QList<ShapePair>() << ShapePair(instr_mem, 0) << ShapePair(ifid, 1) << ShapePair(alu_pc4, 0));
    connPtr = createConnection(alu_pc4, 0, mux_PCSrc, 0);
    connPtr->setFeedbackSettings(false, minConnectionLen, minConnectionLen - 10);
    connPtr->setKinkBias(100);
    createConnection(alu_pc4, 0, ifid, 0);
    createConnection(instr_mem, 0, ifid, 2);

    // ID
    createConnection(ifid, 2,
                     QList<ShapePair>() << ShapePair(registers, 0) << ShapePair(registers, 1) << ShapePair(registers, 2)
                                        << ShapePair(immgen, 0) << /* To IDEX*/
                         ShapePair(idex, 8) << ShapePair(idex, 9) << ShapePair(idex, 10));

    connPtr = createConnection(registers, 0, idex, 5);
    connPtr->setKinkBias(-10);
    connPtr = createConnection(registers, 1, idex, 6);
    connPtr->setKinkBias(10);
    createConnection(immgen, 0, idex, 7);
    connPtr = createConnection(alu_pc_target, 0, mux_PCSrc, 1);
    connPtr->setFeedbackSettings(false, 10, minConnectionLen);
    connPtr->setKinkBias(150);
    createConnection(ifid, 1, alu_pc_target, 0);
    connPtr = createConnection(immgen, 0, alu_pc_target, 1);

    // EX
    createConnection(idex, 4, mux_forwardA_EX, 0);
    createConnection(idex, 5, mux_forwardB_EX, 0);
    createConnection(idex, 3, mux_ALUSrc1, 1);
    createConnection(idex, 6, mux_ALUSrc2, 1);

    createConnection(mux_forwardA_EX, 0, mux_ALUSrc1, 0);
    createConnection(mux_forwardB_EX, 0, mux_ALUSrc2, 0);

    createConnection(mux_ALUSrc1, 0, alu_mainALU, 0);
    createConnection(mux_ALUSrc2, 0, alu_mainALU, 1);

    createConnection(alu_mainALU, 0, exmem, 3);
    connPtr = createConnection(alu_mainALU, 0, mux_PCSrc, 2);
    connPtr->setFeedbackSettings(false, 10, minConnectionLen + 10);
    connPtr->setKinkBias(330);

    // MEM
    createConnection(exmem, 3, mux_alures_PC4_MEM, 0);
    createConnection(exmem, 2, mux_alures_PC4_MEM, 1);

    createConnection(exmem, 0, memwb, 0);
    createConnection(exmem, 2, memwb, 1);
    createConnection(exmem, 3, QList<ShapePair>() << ShapePair(data_mem, 0) << ShapePair(memwb, 3));
    createConnection(exmem, 4, data_mem, 1);
    createConnection(exmem, 5, memwb, 4);
    createConnection(data_mem, 0, memwb, 2);

    connPtr = createConnection(mux_alures_PC4_MEM, 0,
                               QList<ShapePair>() << ShapePair(mux_forwardA_EX, 1) << ShapePair(mux_forwardB_EX, 1));
    connPtr->setFeedbackSettings(true, 10, minConnectionLen + 10);
    connPtr->setKinkBias(150);

    // WB
    createConnection(memwb, 0, mux_memToReg, 0);
    createConnection(memwb, 1, mux_memToReg, 1);
    connPtr = createConnection(mux_memToReg, 0, registers, 3);
    connPtr->setFeedbackSettings(true, minConnectionLen - 20, minConnectionLen - 10);
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
    moveToIO(instr_mem, ifid, instr_mem->getOutputPoint(0), ifid->getInputPoint(2));
    pc->moveBy(instr_mem->sceneBoundingRect().left() - shapeMargin - pc->boundingRect().width(), 0);
    mux_PCSrc->moveBy(pc->sceneBoundingRect().left() - shapeMargin - mux_PCSrc->boundingRect().width(), 0);
    moveToIO(alu_pc4, ifid, alu_pc4->getOutputPoint(0), ifid->getInputPoint(0), 60);

    // position ID stage
    moveToIO(alu_pc_target, idex, alu_pc_target->getOutputPoint(0), idex->getInputPoint(2));
    alu_pc_target->moveBy(-50, 0);
    registers->moveBy(spaceBetweenStateRegs * 0.5, 0);
    moveToIO(immgen, idex, immgen->getOutputPoint(0), idex->getInputPoint(7));
    immgen->moveBy(-120, 20);

    // position EX stage
    moveToIO(alu_mainALU, exmem, alu_mainALU->getOutputPoint(0), exmem->getInputPoint(3));
    moveToIO(mux_ALUSrc1, alu_mainALU, mux_ALUSrc1->getOutputPoint(0), alu_mainALU->getInputPoint(0), 40);
    moveToIO(mux_ALUSrc2, alu_mainALU, mux_ALUSrc2->getOutputPoint(0), alu_mainALU->getInputPoint(1), 40);

    moveToIO(mux_forwardA_EX, mux_ALUSrc1, mux_forwardA_EX->getOutputPoint(0), mux_ALUSrc1->getInputPoint(0), 30);
    moveToIO(mux_forwardB_EX, mux_ALUSrc2, mux_forwardB_EX->getOutputPoint(0), mux_ALUSrc2->getInputPoint(0), 30);

    // position MEM stage
    data_mem->moveBy(spaceBetweenStateRegs * 2.5, 0);
    mux_alures_PC4_MEM->moveBy(spaceBetweenStateRegs * 2.3, 120);

    // position WB stage
    moveToIO(mux_memToReg, memwb, mux_memToReg->getInputPoint(0), memwb->getOutputPoint(0), -minConnectionLen);

    // ----------- Create Etc. objects -----------
    Graphics::DashLine* dash1 = new Graphics::DashLine(ifid);
    Graphics::DashLine* dash2 = new Graphics::DashLine(idex);
    Graphics::DashLine* dash3 = new Graphics::DashLine(exmem);
    Graphics::DashLine* dash4 = new Graphics::DashLine(memwb);

    scene->addItem(dash1);
    scene->addItem(dash2);
    scene->addItem(dash3);
    scene->addItem(dash4);

    int textPad = 20;
    if_instr = new Graphics::Text(QPointF(
        -dash1->sceneBoundingRect().left() + dash1->sceneBoundingRect().width() / 2 + -spaceBetweenStateRegs / 2,
        dash1->sceneBoundingRect().top() + textPad));
    id_instr = new Graphics::Text(
        QPointF(dash1->sceneBoundingRect().left() + dash1->sceneBoundingRect().width() / 2 + spaceBetweenStateRegs / 2,
                dash1->sceneBoundingRect().top() + textPad));
    ex_instr = new Graphics::Text(
        QPointF(dash2->sceneBoundingRect().left() + dash2->sceneBoundingRect().width() / 2 + spaceBetweenStateRegs / 2,
                dash2->sceneBoundingRect().top() + textPad));
    mem_instr = new Graphics::Text(
        QPointF(dash3->sceneBoundingRect().left() + dash3->sceneBoundingRect().width() / 2 + spaceBetweenStateRegs / 2,
                dash3->sceneBoundingRect().top() + textPad));
    wb_instr = new Graphics::Text(QPointF(dash4->sceneBoundingRect().left() + spaceBetweenStateRegs / 3,
                                          dash4->sceneBoundingRect().top() + textPad));

    scene->addItem(if_instr);
    scene->addItem(id_instr);
    scene->addItem(ex_instr);
    scene->addItem(mem_instr);
    scene->addItem(wb_instr);
}

void PipelineWidget::update() {
    QWidget::update();
}

void PipelineWidget::stageTextChanged(Stage stage, const QString& text) {
    switch (stage) {
        case Stage::IF:
            if_instr->setText(text);
            break;
        case Stage::ID:
            id_instr->setText(text);
            break;
        case Stage::EX:
            ex_instr->setText(text);
            break;
        case Stage::MEM:
            mem_instr->setText(text);
            break;
        case Stage::WB:
            wb_instr->setText(text);
            break;
    }
    scene()->update();
}

void PipelineWidget::moveToIO(Graphics::Shape* source, Graphics::Shape* dest, QPointF* sourcePoint, QPointF* destPoint,
                              int connectionLength) {
    // Moves the source shape such that its sourcePoint is aligned with the
    // destination point, seperated by connectionLength
    QPointF sceneSourcePoint = source->mapToScene(*sourcePoint);
    QPointF sceneDestPoint = dest->mapToScene(*destPoint);

    int newY = sceneSourcePoint.y() - sceneDestPoint.y();
    int newX = sceneSourcePoint.x() - sceneDestPoint.x() + connectionLength;

    // Move shapes
    source->moveBy(-newX, -newY);
}

Graphics::Connection* PipelineWidget::createConnection(Graphics::Shape* source, int index1, Graphics::Shape* dest,
                                                       int index2) {
    // Connects a source and destination shape using IO index numbers
    Graphics::Connection* connection =
        new Graphics::Connection(source, source->getOutputPoint(index1), dest, dest->getInputPoint(index2));
    connect(this, &PipelineWidget::displayAllValuesSig, connection, &Graphics::Connection::showValue);
    m_connections.append(connection);
    source->addConnection(connection);
    dest->addConnection(connection);
    scene()->addItem(connection);
    connection->addLabelToScene();
    return connection;
}
Graphics::Connection* PipelineWidget::createConnection(Graphics::Shape* source, int index1, QList<ShapePair> dests) {
    // Connects multiple input points to one output point

    // Generate list of output shapes and points
    QList<QPair<Graphics::Shape*, QPointF*>> shapePointList;
    for (const auto& dest : dests) {
        shapePointList << QPair<Graphics::Shape*, QPointF*>(dest.first, dest.first->getInputPoint(dest.second));
    }

    // Create connection
    Graphics::Connection* connection = new Graphics::Connection(source, source->getOutputPoint(index1), shapePointList);
    connect(this, &PipelineWidget::displayAllValuesSig, connection, &Graphics::Connection::showValue);
    source->addConnection(connection);
    m_connections.append(connection);
    scene()->addItem(connection);
    connection->addLabelToScene();

    // Add connection to destination shapes
    for (const auto& dest : dests) {
        dest.first->addConnection(connection);
    }
    return connection;
}

Graphics::Connection* PipelineWidget::createConnection(Graphics::Shape* source, Graphics::Shape* dest,
                                                       QPointF* sourcePoint, QPointF* destPoint) {
    // Connects source and destinations using point pointers
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
    scaleView(pow((double)2, -event->delta() / 1000.0));
}

void PipelineWidget::zoomIn() {
    scaleView(1.1);
}

void PipelineWidget::zoomOut() {
    scaleView(0.9);
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
