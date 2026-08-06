#pragma once

#include "rvisainfo_common.h"

namespace Ripes {

template <ISA isa>
class ISAVliwInfo : public ISAInfo<isa> {
public:
  ISAVliwInfo(const QStringList extensions) : ISAInfo<isa>(extensions) {
    auto base = static_cast<RVISA::RV_ISAInfoBase*>(this);
    base->unLoadInstructionSet();
    base->m_enabledDetails.insert( RVISA::ImplementationDetail::VliwPseudoInstrExpansion );
    base->loadInstructionSet();
  }
};

}