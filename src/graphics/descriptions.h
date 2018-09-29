#ifndef DESCRIPTIONS_H
#define DESCRIPTIONS_H

#include <QMap>
#include <QString>

namespace Descriptions {
namespace {
static QMap<QString, QString> init() {
    QMap<QString, QString> map;
    map.insert("Registers", "32-entry register file");
    map.insert("Data memory", "Data memory. See memory tab for contents");
    map.insert("Instruction memory", "Instruction memory. See memory tab for contents");
    map.insert("PC", "Program counter");

    map.insert("PC Mux", "Input multiplexer for selecting PC source");
    map.insert("Forward A EX Mux", "Forwarding selector for operand 1");
    map.insert("Forward B EX Mux", "Forwarding selector for operand 2");
    map.insert("Forward A ID Mux", "Forwarding selector for operand 1");
    map.insert("Forward B ID Mux", "Forwarding selector for operand 2");
    map.insert("ALUSrc 1 Mux", "Selects between operand from ID/EX register or a forwarded operand");
    map.insert("ALUSrc 2 Mux", "Selects between operand from ID/EX register or a forwarded operand");
    map.insert("ALURES PC4 Mux",
               "Multiplexer for selecting register forwarding from MEM stage. This can be an ALU result, or the PC "
               "value (AUIPC)");
    map.insert("memToReg Mux", "Selects between ALU result, a read memory value or the program counter value (AUIPC)");

    map.insert("pc4 ALU", "Adds 4 to current PC value");
    map.insert("pc target ALU", "ALU for computing branch target address");
    map.insert("main ALU", "Main functional unit for arithmetic instructions");

    map.insert("immgen", "Immediate value generator");
    map.insert("comp", "Comparator used for branch instructions.\nWhen the ID stage contains a branch instruction,\nthe "
                       "color of the comparator will indicate the comparison result.");

    return map;
}
}

const static QMap<QString, QString> m = init();
}
#endif  // DESCRIPTIONS_H
