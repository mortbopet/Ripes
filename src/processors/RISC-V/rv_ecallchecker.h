#pragma once

#include "Signals/Signal.h"
#include "VSRTL/core/vsrtl_component.h"

#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class EcallChecker : public Component {
public:
    EcallChecker(std::string name, SimComponent* parent) : Component(name, parent) {
        dummy << [=] {
            if (opcode.uValue() == RVInstr::ECALL && !stallEcallHandling.uValue()) {
                m_signal->Emit();
            }
            return 0;
        };

        // This is quite hacky; We need to be able to clear early pipeline registers if an exit syscall has been
        // executed. As such, the outside environment will propagate a call to this EcallChecker which will then infer a
        // signal in the pipeline. This signal may then be used as a method of clearing early pipeline stages while the
        // remainder of the pipeline is emptying the to-be executed instructions after the syscall.
        syscallExit << [=] { return m_syscallExit; };
    }

    void setSysCallSignal(Gallant::Signal0<>* sig) { m_signal = sig; }

    /**
     * @brief setSysCallExiting
     * @note: Must be called _before_ propagating/clocking the circuit, to reflect the change onto the output signal.
     */
    void setSysCallExiting(bool state) { m_syscallExit = state; }
    bool isSysCallExiting() const { return m_syscallExit; }

    Gallant::Signal0<>* m_signal;

    INPUTPORT_ENUM(opcode, RVInstr);

    // Handling an ecall puts an implicit dependence on all outstanding writes to any register being performed before
    // the ecall is handled. As such, the hazard unit may stall handling of the Ecall until all outstanding writes have
    // been perfomed.
    INPUTPORT(stallEcallHandling, 1);

    OUTPUTPORT(dummy, 1);
    OUTPUTPORT(syscallExit, 1);

private:
    bool m_syscallExit = false;
};

}  // namespace core
}  // namespace vsrtl
