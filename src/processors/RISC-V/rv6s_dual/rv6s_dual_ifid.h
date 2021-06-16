#pragma once

#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_ifid.h"

#include "rv6s_dual_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class IFID_DUAL : public IFID<XLEN> {
public:
    IFID_DUAL(std::string name, SimComponent* parent) : IFID<XLEN>(name, parent) {
        CONNECT_REGISTERED_CLEN_INPUT(instr2, this->clear, this->enable);
    }

    REGISTERED_CLEN_INPUT(instr2, XLEN);
};

}  // namespace core
}  // namespace vsrtl
