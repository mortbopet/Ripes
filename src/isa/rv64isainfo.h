#pragma once

#include "isainfo.h"
#include "rvisainfo_common.h"

namespace Ripes {

template <>
class ISAInfo<ISA::RV64I> : public RVISAInfoBase {
public:
    ISAInfo<ISA::RV64I>(const QStringList extensions) {
        // Validate extensions
        for (const auto& ext : extensions) {
            if (supportsExtension(ext)) {
                m_enabledExtensions << ext;
            } else {
                assert(false && "Invalid extension specified for ISA");
            }
        }
    }

    ISA isaID() const override { return ISA::RV64I; }

    unsigned int bits() const override { return 64; }
    QString CCmarch() const override {
        QString march = "rv64i";

        // Proceed in canonical order
        for (const auto& ext : {"M", "A", "F", "D", "C"}) {
            if (m_enabledExtensions.contains(ext)) {
                march += QString(ext).toLower();
            }
        }

        return march;
    }
    QString CCmabi() const override { return "lp64"; }

    unsigned instrByteAlignment() const override { return extensionEnabled("C") ? 2 : 4; };
};

}  // namespace Ripes
