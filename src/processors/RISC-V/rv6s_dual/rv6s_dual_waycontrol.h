#pragma once

#include "../rv_control.h"
#include "VSRTL/core/vsrtl_component.h"
#include "rv6s_dual_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class WayControl : public Component {
private:
    enum class WayClass { Data, Controlflow, Arithmetic, Ecall };

    static bool isControlflow(const VSRTL_VT_U& opcode) {
        return Control::do_jump_ctrl(opcode) || Control::do_branch_ctrl(opcode);
    }

    static bool isLoadStore(const VSRTL_VT_U& opcode) { return Control::do_mem_ctrl(opcode) != +MemOp::NOP; }

    // clang-format off
    static bool isWriteRegInstr(const VSRTL_VT_U& opcode) {
        switch(opcode) {
            case RVInstr::LUI:
            case RVInstr::AUIPC:

            // Arithmetic-immediate instructions
            case RVInstr::ADDI: case RVInstr::SLTI: case RVInstr::SLTIU: case RVInstr::XORI:
            case RVInstr::ORI: case RVInstr::ANDI: case RVInstr::SLLI: case RVInstr::SRLI:
            case RVInstr::SRAI: case RVInstr::ADDIW: case RVInstr::SLLIW: case RVInstr::SRLIW:
            case RVInstr::SRAIW:

            // Arithmetic instructions
            case RVInstr::MUL: case RVInstr::MULH: case RVInstr:: MULHSU: case RVInstr::MULHU:
            case RVInstr::DIV: case RVInstr::DIVU: case RVInstr::REM: case RVInstr::REMU:
            case RVInstr::ADD: case RVInstr::SUB: case RVInstr::SLL: case RVInstr::SLT:
            case RVInstr::SLTU: case RVInstr::XOR: case RVInstr::SRL: case RVInstr::SRA:
            case RVInstr::OR: case RVInstr::AND: case RVInstr::ADDW: case RVInstr::SUBW:
            case RVInstr::SLLW: case RVInstr::SRLW: case RVInstr::SRAW: case RVInstr::MULW:
            case RVInstr::DIVW: case RVInstr::DIVUW: case RVInstr::REMW: case RVInstr::REMUW:

            // Load instructions
            case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU:
            case RVInstr::LHU: case RVInstr::LWU: case RVInstr::LD: case RVInstr::SD:

            // Jump instructions
            case RVInstr::JALR:
            case RVInstr::JAL:
                return true;
            default: return false;
        }
    }
    // clang-format on

    /**
     * @brief structuralHazard
     * Returns true when a structural hazard exists between the two fetched instructions. A structural hazard occurs
     * when both instructions are either a data, control flow or ecall instruction.
     */
    bool structuralHazard(const WayClass& way1Type, const WayClass& way2Type) const {
        return way1Type == way2Type && (way1Type == WayClass::Data || way1Type == WayClass::Controlflow);
    }

    bool rawHazard() const {
        const unsigned wridx_1 = wr_reg_idx_way1.uValue();
        const unsigned wridx_2 = wr_reg_idx_way2.uValue();

        const unsigned idx1_1 = r1_reg_idx_way1.uValue();
        const unsigned idx2_1 = r2_reg_idx_way1.uValue();
        const unsigned idx1_2 = r1_reg_idx_way2.uValue();
        const unsigned idx2_2 = r2_reg_idx_way2.uValue();

        // way1 write => way 2 read?
        const bool hazard_1 =
            (wridx_1 != 0) && (wridx_1 == idx1_2 || wridx_1 == idx2_2) && isWriteRegInstr(opcode_way1.uValue());
        // way2 write => way 1 read?
        const bool hazard_2 =
            (wridx_2 != 0) && (wridx_2 == idx1_1 || wridx_2 == idx2_1) && isWriteRegInstr(opcode_way2.uValue());

        return hazard_1 || hazard_2;
    }

    WayClass instrType(const VSRTL_VT_U opcode) const {
        if (isLoadStore(opcode)) {
            return WayClass ::Data;
        } else if (isControlflow(opcode)) {
            return WayClass::Controlflow;
        } else if (opcode == RVInstr::ECALL) {
            return WayClass::Ecall;
        } else {
            return WayClass::Arithmetic;
        }
    }

    WaySrc otherWay(const WaySrc way) const { return way == +WaySrc::WAY1 ? WaySrc::WAY2 : WaySrc::WAY1; }

    /**
     * @brief computeCycle
     * Computes internal information which may be used in the calculation of all output ports of this component.
     */
    void computeCycle() {
        if (m_design->getCycleCount() == cachedCycle)
            return;

        // Default assignments
        const WayClass way1Type = instrType(opcode_way1.uValue());
        const WayClass way2Type = instrType(opcode_way2.uValue());

        const bool stallNow = stall_in.uValue();
        if (stallNow) {
            // Stall implies that the 2nd fetched instruction must be issued.
            m_dataWayValid = way2Type == WayClass::Data;
            m_dataWaySrc = WaySrc::WAY2;
            m_execWayValid = way2Type != WayClass::Data;
            m_execWaySrc = WaySrc::WAY2;
            m_stall = false;
        } else if (way1Type == WayClass::Controlflow) {
            // Control flow hazard; only issue 1st fetched instruction
            m_dataWayValid = false;
            m_execWayValid = true;
            m_execWaySrc = WaySrc::WAY1;
            m_stall = true && ifid_valid.uValue();
        } else if (structuralHazard(way1Type, way2Type) || way2Type == WayClass::Ecall || way1Type == WayClass::Ecall) {
            // Structural hazard or ecall; always issue way 1 instruction (execute in-order)
            m_dataWayValid = way1Type == WayClass::Data;
            m_dataWaySrc = WaySrc::WAY1;
            m_execWayValid = way1Type != WayClass::Data;
            m_execWaySrc = WaySrc::WAY1;
            m_stall = true && ifid_valid.uValue();
        } else if (rawHazard()) {
            // WAR hazard; only issue 1st fetched instruction
            m_dataWayValid = way1Type == WayClass::Data;
            m_execWayValid = way1Type != WayClass::Data;
            m_dataWaySrc = WaySrc::WAY1;
            m_execWaySrc = WaySrc::WAY1;
            m_stall = true && ifid_valid.uValue();
        } else {
            // Can issue both
            m_dataWayValid = true;
            m_execWayValid = true;

            if (way1Type == WayClass::Data || way2Type == WayClass::Data) {
                m_dataWaySrc = way1Type == WayClass::Data ? WaySrc::WAY1 : WaySrc::WAY2;
                m_execWaySrc = otherWay(m_dataWaySrc);
            } else if ((way1Type == WayClass::Controlflow || way1Type == WayClass::Ecall) ||
                       (way2Type == WayClass::Controlflow || way2Type == WayClass::Ecall)) {
                m_execWaySrc =
                    (way1Type == WayClass::Controlflow || way1Type == WayClass::Ecall) ? WaySrc::WAY1 : WaySrc::WAY2;
                m_dataWaySrc = otherWay(m_execWaySrc);
            } else {
                // Both arithmetic instructions; issue in any of the two ways
                m_dataWaySrc = WaySrc::WAY1;
                m_execWaySrc = WaySrc::WAY2;
            }

            m_stall = false;
            Q_ASSERT(m_dataWaySrc != m_execWaySrc);
        }

        cachedCycle = m_design->getCycleCount();
    }

public:
    WayControl(const std::string& name, SimComponent* parent) : Component(name, parent) {
        data_way_valid << [=] {
            computeCycle();
            return m_dataWayValid;
        };
        exec_way_valid << [=] {
            computeCycle();
            return m_execWayValid;
        };
        data_way_src << [=] {
            computeCycle();
            return m_dataWaySrc;
        };
        exec_way_src << [=] {
            computeCycle();
            return m_execWaySrc;
        };

        stall_out << [=] {
            computeCycle();
            return m_stall;
        };

        m_design = getDesign();
        Q_ASSERT(m_design != nullptr);
    }

    INPUTPORT(ifid_valid, 1);

    INPUTPORT_ENUM(opcode_way1, RVInstr);
    INPUTPORT_ENUM(opcode_way2, RVInstr);

    INPUTPORT(r1_reg_idx_way1, c_RVRegsBits);
    INPUTPORT(r2_reg_idx_way1, c_RVRegsBits);
    INPUTPORT(r1_reg_idx_way2, c_RVRegsBits);
    INPUTPORT(r2_reg_idx_way2, c_RVRegsBits);

    INPUTPORT(wr_reg_idx_way1, c_RVRegsBits);
    INPUTPORT(wr_reg_idx_way2, c_RVRegsBits);

    OUTPUTPORT_ENUM(data_way_src, WaySrc);
    OUTPUTPORT_ENUM(exec_way_src, WaySrc);

    OUTPUTPORT(exec_way_valid, 1);
    OUTPUTPORT(data_way_valid, 1);

    // Stall when only able to schedule 1 out of the 2 fetched instructions
    INPUTPORT(stall_in, 1);
    OUTPUTPORT(stall_out, 1);

private:
    SimDesign* m_design;

    unsigned cachedCycle = -1;

    // Cached data for next-state computation
    bool m_dataWayValid;
    bool m_execWayValid;
    WaySrc m_execWaySrc = WaySrc::WAY1;
    WaySrc m_dataWaySrc = WaySrc::WAY1;
    bool m_stall = false;
};

}  // namespace core
}  // namespace vsrtl
