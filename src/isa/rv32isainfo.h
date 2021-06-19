#pragma once

#include "isainfo.h"
#include "rvisainfo_common.h"

namespace Ripes {

template <>
class ISAInfo<ISA::RV32I> : public RVISAInfoBase {
public:
    ISAInfo<ISA::RV32I>(const QStringList extensions) {
        // Validate extensions
        for (const auto& ext : extensions) {
            if (supportsExtension(ext)) {
                m_enabledExtensions << ext;
            } else {
                assert(false && "Invalid extension specified for ISA");
            }
        }
    }

    QString name() const override { return "RV32I"; }
    ISA isaID() const override { return ISA::RV32I; }

    unsigned int bits() const override { return 32; }
    unsigned elfMachineId() const override { return EM_RISCV; }
    QString CCmarch() const override {
        QString march = "rv32i";

        // Proceed in canonical order
        for (const auto& ext : {"M", "A", "F", "D"}) {
            if (m_enabledExtensions.contains(ext)) {
                march += QString(ext).toLower();
            }
        }

        return march;
    }
    QString CCmabi() const override { return "ilp32"; }
};

}  // namespace Ripes
