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
            if (opcode.uValue() == RVInstr::ECALL && !stallEcallHandling.uValue() && !handlingEcall) {
                handlingEcall = true;
                m_signal->Emit();
                handlingEcall = false;
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

    // While an ECALL is being handled, the handling of the ECALL may result in the register file being written
    // (ie. return arguments being inserted into the register file). When this occurs, the circuit is
    // repropagated. This repropagation will inherently retrigger execution of the ecall which caused the trap. To avoid
    // this loop, we ensure that the ecall handling signal can never be emitted whilst the emit call hasn't returned
    // (wherein returning from the signal emission indicates finished ecall handling from the execution environment).
    bool handlingEcall = false;
};

}  // namespace core
}  // namespace vsrtl
