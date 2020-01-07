#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "Signals/Signal.h"

#include "riscv.h"

namespace vsrtl {
using namespace core;
namespace RISCV {

class EcallChecker : public Component {
public:
    EcallChecker(std::string name, SimComponent* parent) : Component(name, parent) {
        dummy << [=] {
            if(opcode.uValue() == RVInstr::ECALL){
                m_signal->Emit();
            }
            return 0;
        };
    }

    void setSysCallSignal(Gallant::Signal0<>* sig){
        m_signal = sig;
    }

    Gallant::Signal0<>* m_signal;


    INPUTPORT_ENUM(opcode, RVInstr);
    OUTPUTPORT(dummy, 1);
};

}
}
