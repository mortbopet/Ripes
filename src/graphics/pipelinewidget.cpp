#include "pipelinewidget.h"
#include "backgrounditems.h"
#include "connection.h"
#include "shape.h"

#include "descriptions.h"

#include <QRectF>
#include <cmath>

#include <QCoreApplication>
#include <QWheelEvent>

qreal PipelineWidget::spaceBetweenStateRegs = 380;

PipelineWidget::PipelineWidget(QWidget* parent) : QGraphicsView(parent) {
    auto* scene = new QGraphicsScene(this);
    // scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);

    m_pipelinePtr = Pipeline::getPipeline();

    //  ------------ Create pipeline objects ---------------
    // IMPORTANT! all animated items (items with signals connected to them) MUST be added to m_animatedItems for its
    // update function to be triggered after a simulator step

    // Registers memory
    auto* registers = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::ID, 10, 15);
    registers->addInput(QStringList() << "Read\nregister 1"
                                      << "Read\nregister 2"
                                      << "Write\nregister"
                                      << "Write\ndata");
    registers->addOutput(QStringList() << "Read\ndata 1"
                                       << "Read\ndata 2");
    registers->addTopPoint("Write");
    registers->setName("Registers");
    registers->setSignal(Graphics::SignalPos::Top, m_pipelinePtr->r_regWrite_MEMWB.getOutput());
    m_animatedItems.push_back(registers);
    registers->setToolTip(Descriptions::m["Registers"]);

    // Data memory
    auto* data_mem = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::MEM, 50, 5);
    data_mem->addInput(QStringList() << "Address"
                                     << "Write\ndata");
    data_mem->addOutput(QStringList() << "Read\ndata");
    data_mem->addTopPoint("Write");
    data_mem->addBotPoint("Read");
    data_mem->setName("Data\nmemory");
    data_mem->setSignal(Graphics::SignalPos::Top, m_pipelinePtr->r_MemWrite_EXMEM.getOutput());
    data_mem->setSignal(Graphics::SignalPos::Bottom, m_pipelinePtr->r_MemRead_EXMEM.getOutput());
    m_animatedItems.push_back(data_mem);
    data_mem->setToolTip(Descriptions::m["Data memory"]);

    // Instruction memory
    auto* instr_mem = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::IF, 60, 0);
    instr_mem->addInput(QStringList() << "");
    instr_mem->addOutput(QStringList() << "");
    instr_mem->setName("Instruction\nmemory");
    instr_mem->setToolTip(Descriptions::m["Instruction memory"]);

    // PC
    auto* pc = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::IF, 30, 3);
    pc->addInput();
    pc->addOutput();
    pc->addBotPoint("");
    pc->setSignal(Graphics::SignalPos::Bottom, &m_pipelinePtr->s_PCWrite);
    pc->setName("PC");
    m_animatedItems.push_back(pc);
    pc->setToolTip(Descriptions::m["PC"]);

    // MUXes
    auto* mux_PCSrc = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::IF, 0, 0);
    mux_PCSrc->addInput(QStringList() << ""
                                      << ""
                                      << "");
    mux_PCSrc->addOutput();
    mux_PCSrc->setName("M\nu\nx");
    mux_PCSrc->setSignal(Graphics::SignalPos::Left, &m_pipelinePtr->s_PCSrc);
    m_animatedItems.push_back(mux_PCSrc);
    mux_PCSrc->setToolTip(Descriptions::m["PC Mux"]);

    auto* mux_forwardA_EX = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, -15, 0);
    mux_forwardA_EX->addInput(QStringList() << ""
                                            << ""
                                            << "");
    mux_forwardA_EX->addOutput();
    mux_forwardA_EX->setName("M\nu\nx");
    mux_forwardA_EX->setSignal(Graphics::SignalPos::Left, &m_pipelinePtr->s_forwardA_EX);
    m_animatedItems.push_back(mux_forwardA_EX);
    mux_forwardA_EX->setToolTip(Descriptions::m["Forward A EX Mux"]);

    auto* mux_forwardB_EX = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, -15, 0);
    mux_forwardB_EX->addInput(QStringList() << ""
                                            << ""
                                            << "");
    mux_forwardB_EX->addOutput();
    mux_forwardB_EX->setName("M\nu\nx");
    mux_forwardB_EX->setSignal(Graphics::SignalPos::Left, &m_pipelinePtr->s_forwardB_EX);
    m_animatedItems.push_back(mux_forwardB_EX);
    mux_forwardB_EX->setToolTip(Descriptions::m["Forward B EX Mux"]);

    auto* mux_forwardA_ID = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, -15, 0);
    mux_forwardA_ID->addInput(QStringList() << ""
                                            << ""
                                            << "");
    mux_forwardA_ID->addOutput();
    mux_forwardA_ID->setName("M\nu\nx");
    mux_forwardA_ID->setSignal(Graphics::SignalPos::Left, &m_pipelinePtr->s_forwardA_ID);
    m_animatedItems.push_back(mux_forwardA_ID);
    mux_forwardA_ID->setToolTip(Descriptions::m["Forward A ID Mux"]);

    auto* mux_forwardB_ID = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, -15, 0);
    mux_forwardB_ID->addInput(QStringList() << ""
                                            << ""
                                            << "");
    mux_forwardB_ID->addOutput();
    mux_forwardB_ID->setName("M\nu\nx");
    mux_forwardB_ID->setSignal(Graphics::SignalPos::Left, &m_pipelinePtr->s_forwardB_ID);
    m_animatedItems.push_back(mux_forwardB_ID);
    mux_forwardB_ID->setToolTip(Descriptions::m["Forward B ID Mux"]);

    auto* mux_ALUSrc1 = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 0, 0);
    mux_ALUSrc1->addInput(QStringList() << ""
                                        << "");
    mux_ALUSrc1->addOutput();
    mux_ALUSrc1->setName("M\nu\nx");
    mux_ALUSrc1->setSignal(Graphics::SignalPos::Left, m_pipelinePtr->r_ALUSrc1_IDEX.getOutput());
    m_animatedItems.push_back(mux_ALUSrc1);
    mux_ALUSrc1->setToolTip(Descriptions::m["ALUSrc 1 Mux"]);

    auto* mux_ALUSrc2 = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 0, 0);
    mux_ALUSrc2->addInput(QStringList() << ""
                                        << "");
    mux_ALUSrc2->addOutput();
    mux_ALUSrc2->setName("M\nu\nx");
    mux_ALUSrc2->setSignal(Graphics::SignalPos::Left, m_pipelinePtr->r_ALUSrc2_IDEX.getOutput());
    m_animatedItems.push_back(mux_ALUSrc2);
    mux_ALUSrc2->setToolTip(Descriptions::m["ALUSrc 2 Mux"]);

    auto* mux_alures_PC4_MEM = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::EX, 0, 0);
    mux_alures_PC4_MEM->addInput(QStringList() << ""
                                               << "");
    mux_alures_PC4_MEM->addOutput();
    mux_alures_PC4_MEM->setName("M\nu\nx");
    mux_alures_PC4_MEM->setSignal(Graphics::SignalPos::Left, &m_pipelinePtr->s_alures_PC4_MEM);
    m_animatedItems.push_back(mux_alures_PC4_MEM);
    mux_alures_PC4_MEM->setToolTip(Descriptions::m["ALURES PC4 Mux"]);

    auto* mux_memToReg = new Graphics::Shape(Graphics::ShapeType::MUX, Graphics::Stage::WB, 0, 0);
    mux_memToReg->addInput(QStringList() << ""
                                         << ""
                                         << "");
    mux_memToReg->addOutput();
    mux_memToReg->setName("M\nu\nx");
    mux_memToReg->setSignal(Graphics::SignalPos::Left, m_pipelinePtr->r_memToReg_MEMWB.getOutput());
    m_animatedItems.push_back(mux_memToReg);
    mux_memToReg->setToolTip(Descriptions::m["memToReg Mux"]);

    // ALUs
    auto* alu_pc4 = new Graphics::Shape(Graphics::ShapeType::ALU, Graphics::Stage::ID, 70, 10);
    alu_pc4->setName("+");
    alu_pc4->addOutput();
    alu_pc4->addInput(QStringList() << ""
                                    << " 4");
    alu_pc4->setToolTip(Descriptions::m["pc4 ALU"]);

    auto* alu_pc_target = new Graphics::Shape(Graphics::ShapeType::ALU, Graphics::Stage::EX, 70, 10);
    alu_pc_target->setName("+");
    alu_pc_target->addOutput();
    alu_pc_target->setToolTip(Descriptions::m["pc target ALU"]);

    auto* alu_mainALU = new Graphics::Shape(Graphics::ShapeType::ALU, Graphics::Stage::EX, 135, 10);
    alu_mainALU->setName("ALU");
    alu_mainALU->addOutput();
    alu_mainALU->setToolTip(Descriptions::m["main ALU"]);

    // State registers
    ifid = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::ID, 0, 10);
    ifid->setName("IF/ID");
    ifid->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 3; i++) {
        ifid->addInput();
        ifid->addOutput();
    }
    ifid->addInput("Enable");
    ifid->addInput("Reset\n(sync.)");
    ifid->addOutput();
    ifid->addOutput();
    ifid->setHiddenOutputs(std::set<int>{3, 4});
    // Set signals for Write and Reset
    ifid->setSingleIOBlink(true);
    ifid->addIOSignalPair(3, &m_pipelinePtr->s_IFID_write);
    ifid->addIOSignalPair(4, &m_pipelinePtr->s_IFID_reset);
    m_animatedItems.push_back(ifid);

    idex = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::EX, 0, 10);
    idex->setName("ID/EX");
    idex->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 11; i++) {
        idex->addInput();
        idex->addOutput();
    }
    idex->addInput("Reset\n(sync.)");
    idex->addOutput();
    idex->setHiddenOutputs(std::set<int>{1, 2, 8, 10 ,11});
    idex->setHiddenInputs(std::set<int>{0, 1, 2,8,10});
    idex->setSingleIOBlink(true);
    idex->addIOSignalPair(11, &m_pipelinePtr->s_IDEX_reset);
    m_animatedItems.push_back(idex);

    exmem = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::EX, 0, 10);
    exmem->setName("EX/MEM");
    exmem->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 6; i++) {
        exmem->addInput();
        exmem->addOutput();
    }
    exmem->setHiddenInputs(std::set<int>{1});
    exmem->setHiddenOutputs(std::set<int>{1});

    memwb = new Graphics::Shape(Graphics::ShapeType::Block, Graphics::Stage::EX, 0, 10);
    memwb->setName("MEM/WB");
    memwb->setFixedHeight(true, stateRegHeight);
    for (int i = 0; i < 5; i++) {
        memwb->addInput();
        memwb->addOutput();
    }

    // Static items
    auto* immgen = new Graphics::Shape(Graphics::ShapeType::Static, Graphics::Stage::EX, 20, 0);
    immgen->setName("Imm\ngen");
    immgen->addInput();
    immgen->addOutput();
    immgen->setToolTip(Descriptions::m["immgen"]);

    auto* comp = new Graphics::Shape(Graphics::ShapeType::Comparator, Graphics::Stage::EX, 0, 20);
    comp->addOutput();
    comp->setHiddenOutputs(std::set<int>{0});
    comp->addTopPoint("");
    comp->addBotPoint("");
    comp->setToolTip(Descriptions::m["comp"]);
    comp->setSignal(Graphics::SignalPos::Bottom, &m_pipelinePtr->s_branchTaken);
    comp->setSignal(Graphics::SignalPos::Top, &m_pipelinePtr->s_CompOp);
    comp->setSignal(Graphics::SignalPos::Left, &m_pipelinePtr->s_Branch);
    m_animatedItems.push_back(comp);

    //  ----------- Create connections ----------------------
    Graphics::Connection* connPtr;

    // IF
    createConnection(mux_PCSrc, 0, pc, 0, m_pipelinePtr->mux_PCSrc.getOutput());
    connPtr = createConnection(
        pc, 0, QList<ShapePair>() << ShapePair(instr_mem, 0) << ShapePair(ifid, 1) << ShapePair(alu_pc4, 0));
    setSignal(connPtr, m_pipelinePtr->r_PC_IF.getOutput());
    createConnection(instr_mem, 0, ifid, 2, &m_pipelinePtr->s_instr_IF);
    createConnection(alu_pc4, 0, ifid, 0);
    connPtr = createConnection(alu_pc4, 0, mux_PCSrc, 0, m_pipelinePtr->alu_pc4.getOutput());
    connPtr->setFeedbackSettings(false, minConnectionLen, minConnectionLen - 10);
    connPtr->setKinkBias(-70);

    // ID
    connPtr = createConnection(ifid, 2,
                               QList<ShapePair>() << ShapePair(registers, 0) << ShapePair(registers, 1)
                                                  << ShapePair(immgen, 0) << /* To IDEX*/ ShapePair(idex, 9));
    setSignal(connPtr, m_pipelinePtr->r_instr_IFID.getOutput());
    connPtr->setKinkBias(15);

    // createConnection(registers, 0, idex, 5)->setKinkBias(-10);
    createConnection(registers, 0, mux_forwardA_ID, 0)->setKinkBias(-10)->drawPointAtFirstKink(true);
    createConnection(registers, 0, idex, 5, &m_pipelinePtr->s_readRegister1);
    createConnection(registers, 1, mux_forwardB_ID, 0)->drawPointAtFirstKink(true);
    createConnection(registers, 1, idex, 6, &m_pipelinePtr->s_readRegister2);

    createConnection(immgen, 0, alu_pc_target, 1)->setKinkBias(20)->drawPointAtFirstKink(true);
    createConnection(immgen, 0, idex, 7, &m_pipelinePtr->s_imm_ID)->setKinkBias(25);
    connPtr = createConnection(alu_pc_target, 0, mux_PCSrc, 1, m_pipelinePtr->alu_pc_target.getOutput());
    connPtr->setFeedbackSettings(false, 10, minConnectionLen);
    connPtr->setKinkBias(-150);
    connPtr = createConnection(ifid, 1, QList<ShapePair>() << ShapePair(alu_pc_target, 0) << ShapePair(idex, 4))
                  ->setKinkBias(20);
    setSignal(connPtr, m_pipelinePtr->r_PC_IFID.getOutput());
    createConnection(ifid, 0, idex, 3, m_pipelinePtr->r_PC4_IDEX.getOutput())->setKinkBias(-110);

    connPtr = createConnection(mux_forwardA_ID, comp, mux_forwardA_ID->getOutputPoint(0), comp->getTopPoint())
                  ->setDirection(Graphics::Direction::south);
    setSignal(connPtr, m_pipelinePtr->mux_forwardA_ID.getOutput());
    connPtr = createConnection(mux_forwardB_ID, comp, mux_forwardB_ID->getOutputPoint(0), comp->getBotPoint())
                  ->setDirection(Graphics::Direction::north);
    setSignal(connPtr, m_pipelinePtr->mux_forwardB_ID.getOutput());

    // EX

    createConnection(idex, 0, exmem, 0, m_pipelinePtr->r_regWrite_IDEX.getOutput());
    createConnection(mux_forwardA_EX, 0, mux_ALUSrc1, 0, m_pipelinePtr->mux_forwardA_EX.getOutput());
    connPtr =
        createConnection(mux_forwardB_EX, 0, QList<ShapePair>() << ShapePair(mux_ALUSrc2, 0) << ShapePair(exmem, 4));
    setSignal(connPtr, m_pipelinePtr->mux_forwardB_EX.getOutput());

    createConnection(mux_ALUSrc1, 0, alu_mainALU, 0, m_pipelinePtr->mux_ALUSrc1.getOutput());
    createConnection(mux_ALUSrc2, 0, alu_mainALU, 1, m_pipelinePtr->mux_ALUSrc2.getOutput());

    createConnection(alu_mainALU, 0, exmem, 3, m_pipelinePtr->alu_mainALU.getOutput());
    connPtr = createConnection(alu_mainALU, 0, mux_PCSrc, 2);
    connPtr->setFeedbackSettings(false, 10, minConnectionLen);
    connPtr->setKinkBias(325);
    createConnection(idex, 5, mux_forwardA_EX, 0, m_pipelinePtr->r_readRegister1_IDEX.getOutput());
    createConnection(idex, 6, mux_forwardB_EX, 0, m_pipelinePtr->r_readRegister2_IDEX.getOutput());
    createConnection(idex, 7, mux_ALUSrc2, 1, m_pipelinePtr->r_imm_IDEX.getOutput())->setKinkBias(-50);
    createConnection(idex, 3, exmem, 2, m_pipelinePtr->r_PC4_IDEX.getOutput())->setKinkBias(100);
    createConnection(idex, 4, mux_ALUSrc1, 1, m_pipelinePtr->r_PC_IDEX.getOutput())->setKinkBias(40);
    createConnection(idex, 9, exmem, 5, m_pipelinePtr->r_writeReg_IDEX.getOutput());

    // MEM
    // createConnection(exmem, 3, mux_alures_PC4_MEM, 0, m_pipelinePtr->r_alures_EXMEM.getOutput());
    createConnection(exmem, 2, mux_alures_PC4_MEM, 1, m_pipelinePtr->r_PC4_EXMEM.getOutput())
        ->setKinkBias(-15)
        ->drawPointAtFirstKink(true);

    createConnection(exmem, 0, memwb, 0, m_pipelinePtr->r_regWrite_EXMEM.getOutput());
    createConnection(exmem, 2, memwb, 1)->setKinkBias(-80);
    connPtr = createConnection(exmem, 3,
                               QList<ShapePair>()
                                   << ShapePair(data_mem, 0) << ShapePair(memwb, 3) << ShapePair(mux_alures_PC4_MEM, 0))
                  ->setKinkBias(10);
    // setSignal(connPtr, m_pipelinePtr->r_read);
    createConnection(exmem, 4, data_mem, 1, m_pipelinePtr->r_writeData_EXMEM.getOutput())->setKinkBias(-20);
    createConnection(exmem, 5, memwb, 4, m_pipelinePtr->r_writeReg_EXMEM.getOutput());
    createConnection(data_mem, 0, memwb, 2, &m_pipelinePtr->readData_MEM);

    connPtr = createConnection(mux_alures_PC4_MEM, 0,
                               QList<ShapePair>() << ShapePair(mux_forwardA_EX, 1) << ShapePair(mux_forwardB_EX, 1));

    connPtr->setFeedbackSettings(true, 10, minConnectionLen - 10);
    connPtr->setKinkBias(200);
    connPtr = createConnection(mux_alures_PC4_MEM, 0,
                               QList<ShapePair>() << ShapePair(mux_forwardA_ID, 1) << ShapePair(mux_forwardB_ID, 1));

    connPtr->setFeedbackSettings(true, 10, minConnectionLen - 10);
    connPtr->setKinkBias(200);

    // WB
    connPtr = createConnection(memwb, registers, memwb->getOutputPoint(0), registers->getTopPoint(), m_pipelinePtr->r_regWrite_MEMWB.getOutput());
    connPtr->setFeedbackSettings(true, minConnectionLen - 20, minConnectionLen - 10);
    connPtr->setKinkBias(-100);
    connPtr->setDirection(Graphics::Direction::south);
    createConnection(memwb, 2, mux_memToReg, 0, m_pipelinePtr->r_readData_MEMWB.getOutput());
    createConnection(memwb, 1, mux_memToReg, 2, m_pipelinePtr->r_PC4_MEMWB.getOutput());
    createConnection(memwb, 3, mux_memToReg, 1, m_pipelinePtr->r_alures_MEMWB.getOutput())->setKinkBias(-10);

    connPtr = createConnection(mux_memToReg, 0,
                               QList<ShapePair>() << ShapePair(mux_forwardA_EX, 2) << ShapePair(mux_forwardB_EX, 2));
    connPtr->setFeedbackSettings(false, minConnectionLen - 20, minConnectionLen + 20);
    connPtr->setKinkBias(320);

    connPtr = createConnection(mux_memToReg, 0,
                               QList<ShapePair>() << ShapePair(mux_forwardA_ID, 2) << ShapePair(mux_forwardB_ID, 2));
    connPtr->setFeedbackSettings(false, minConnectionLen - 20, minConnectionLen + 20);
    connPtr->setKinkBias(320);

    connPtr = createConnection(mux_memToReg, 0, registers, 3, m_pipelinePtr->mux_memToReg.getOutput());
    connPtr->setFeedbackSettings(false, minConnectionLen - 20, minConnectionLen - 5);
    connPtr->setKinkBias(320);

    connPtr = createConnection(memwb, 4, registers, 2);
    connPtr->setFeedbackSettings(false, minConnectionLen - 20, minConnectionLen);
    connPtr->setKinkBias(210);
    setSignal(connPtr, m_pipelinePtr->r_writeReg_MEMWB.getOutput());

    // ------- ITEM POSITIONING ------
    // Item positioning will mostly be done manually.
    // Most of it is based on positioning an "anchor" item, and calculating
    // other
    // items position relative to these

    // Position state registers
    ifid->moveBy(-spaceBetweenStateRegs * 0.22, 0);
    idex->moveBy(spaceBetweenStateRegs * 1, 0);
    exmem->moveBy(spaceBetweenStateRegs * 2.05, 0);
    memwb->moveBy(spaceBetweenStateRegs * 3.05, 0);

    // position IF stage
    moveToIO(instr_mem, ifid, instr_mem->getOutputPoint(0), ifid->getInputPoint(2));
    pc->moveBy(instr_mem->sceneBoundingRect().left() - shapeMargin - pc->boundingRect().width(), 0);
    mux_PCSrc->moveBy(pc->sceneBoundingRect().left() - shapeMargin - mux_PCSrc->boundingRect().width(), 0);
    moveToIO(alu_pc4, ifid, alu_pc4->getOutputPoint(0), ifid->getInputPoint(0), 60);

    // position ID stage
    moveToIO(alu_pc_target, idex, alu_pc_target->getOutputPoint(0), idex->getInputPoint(2));
    alu_pc_target->moveBy(8, -30);
    registers->moveBy(spaceBetweenStateRegs * 0.3 - 10, 20);
    moveToIO(immgen, idex, immgen->getOutputPoint(0), idex->getInputPoint(7));
    immgen->moveBy(-120, 65);

    moveToIO(mux_forwardA_ID, idex, mux_forwardA_ID->getOutputPoint(0), idex->getInputPoint(11));
    mux_forwardA_ID->moveBy(-15, -10);
    moveToIO(mux_forwardB_ID, idex, mux_forwardB_ID->getOutputPoint(0), idex->getInputPoint(11));
    mux_forwardB_ID->moveBy(-15, 80);
    moveToIO(comp, idex, comp->getOutputPoint(0), idex->getInputPoint(11));
    comp->moveBy(25, 30);

    // position EX stage
    moveToIO(alu_mainALU, exmem, alu_mainALU->getOutputPoint(0), exmem->getInputPoint(3));
    moveToIO(mux_ALUSrc1, alu_mainALU, mux_ALUSrc1->getOutputPoint(0), alu_mainALU->getInputPoint(0), 40);
    moveToIO(mux_ALUSrc2, alu_mainALU, mux_ALUSrc2->getOutputPoint(0), alu_mainALU->getInputPoint(1), 40);

    moveToIO(mux_forwardA_EX, mux_ALUSrc1, mux_forwardA_EX->getOutputPoint(0), mux_ALUSrc1->getInputPoint(0), 30);
    moveToIO(mux_forwardB_EX, mux_ALUSrc2, mux_forwardB_EX->getOutputPoint(0), mux_ALUSrc2->getInputPoint(0), 30);

    // position MEM stage
    data_mem->moveBy(spaceBetweenStateRegs * 2.63, 0);
    mux_alures_PC4_MEM->moveBy(spaceBetweenStateRegs * 2.4, 130);

    // position WB stage
    moveToIO(mux_memToReg, memwb, mux_memToReg->getInputPoint(0), memwb->getOutputPoint(2), -minConnectionLen);

    // ----------- Create Etc. objects -----------
    auto* dash1 = new Graphics::DashLine(ifid);
    auto* dash2 = new Graphics::DashLine(idex);
    auto* dash3 = new Graphics::DashLine(exmem);
    auto* dash4 = new Graphics::DashLine(memwb);

    scene->addItem(dash1);
    scene->addItem(dash2);
    scene->addItem(dash3);
    scene->addItem(dash4);

    int textPad = 20;
    if_instr = new Graphics::Text(
        QPointF(-dash1->sceneBoundingRect().left() + dash1->sceneBoundingRect().width() / 2 + -spaceBetweenStateRegs,
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
    wb_instr = new Graphics::Text(QPointF(dash4->sceneBoundingRect().left() + spaceBetweenStateRegs / 3 + 20,
                                          dash4->sceneBoundingRect().top() + textPad));

    scene->addItem(if_instr);
    scene->addItem(id_instr);
    scene->addItem(ex_instr);
    scene->addItem(mem_instr);
    scene->addItem(wb_instr);

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
    scene->addItem(mux_forwardA_ID);
    scene->addItem(mux_forwardB_ID);
    scene->addItem(mux_ALUSrc1);
    scene->addItem(mux_ALUSrc2);
    scene->addItem(mux_alures_PC4_MEM);
    scene->addItem(mux_memToReg);
    scene->addItem(alu_pc_target);
    scene->addItem(alu_mainALU);
    scene->addItem(idex);
    scene->addItem(exmem);
    scene->addItem(memwb);
    scene->addItem(comp);

    // Add labels to scene (on top of all items and connections)
    for (auto& conn : m_connections) {
        conn->addLabelToScene();
        // Calculate connection paths
        conn->finalize();
    }

    // For some reason, the boundingrect of the leftmost connections are not forcing the scene rect to change
    // We manually adjust it a bit
    setSceneRect(scene->itemsBoundingRect().adjusted(-50, -50, 50, 100));
}

void PipelineWidget::notifyLabelsGeometryChange(){
    // Since labels geometry can change from clock cycle to clock cycle,
    // make connections call prepareGeometryChange on their labels
    for(const auto& c : m_connections){
        c->updateLabel();
    }
}

void PipelineWidget::update() {
    if(!m_pipelinePtr->isRunning()){
        for (const auto& item : m_animatedItems) {
            notifyLabelsGeometryChange();
            item->update();
        }
    }
}

void PipelineWidget::setSignal(Graphics::Connection* conn, SignalBase* sig) {
    conn->setSignal(sig);
    m_animatedItems.push_back(conn->getLabel());
}

void PipelineWidget::stageTextChanged(Stage stage, const QString& text, QColor col) {
    switch (stage) {
        case Stage::IF:
            if_instr->setText(text, col);
            break;
        case Stage::ID:
            id_instr->setText(text, col);
            break;
        case Stage::EX:
            ex_instr->setText(text, col);
            break;
        case Stage::MEM:
            mem_instr->setText(text, col);
            break;
        case Stage::WB:
            wb_instr->setText(text, col);
            break;
    }
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
                                                       int index2, SignalBase* sig) {
    // Connects a source and destination shape using IO index numbers
    auto* connection =
        new Graphics::Connection(source, source->getOutputPoint(index1), dest, dest->getInputPoint(index2));
    connect(this, &PipelineWidget::displayAllValuesSig, connection, &Graphics::Connection::toggleLabel);

    m_connections.append(connection);
    source->addConnection(connection);
    dest->addConnection(connection);
    scene()->addItem(connection);

    // If a signal has been specified, set the connection to this signal, and make the label updating
    if (sig != nullptr) {
        connection->setSignal(sig);
        m_animatedItems.push_back(connection->getLabel());
    }

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
    connect(this, &PipelineWidget::displayAllValuesSig, connection, &Graphics::Connection::toggleLabel);
    source->addConnection(connection);
    m_connections.append(connection);
    scene()->addItem(connection);

    // Add connection to destination shapes
    for (const auto& dest : dests) {
        dest.first->addConnection(connection);
    }
    return connection;
}

Graphics::Connection* PipelineWidget::createConnection(Graphics::Shape* source, Graphics::Shape* dest,
                                                       QPointF* sourcePoint, QPointF* destPoint, SignalBase* sig) {
    // Connects source and destinations using point pointers
    auto* connection = new Graphics::Connection(source, sourcePoint, dest, destPoint);
    connect(this, &PipelineWidget::displayAllValuesSig, connection, &Graphics::Connection::toggleLabel);


    m_connections.append(connection);
    source->addConnection(connection);
    dest->addConnection(connection);
    scene()->addItem(connection);

    // If a signal has been specified, set the connection to this signal, and make the label updating
    if (sig != nullptr) {
        connection->setSignal(sig);
        m_animatedItems.push_back(connection->getLabel());
    }


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
    scaleView(std::pow((double)2, -event->delta() / 1000.0));
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
}
