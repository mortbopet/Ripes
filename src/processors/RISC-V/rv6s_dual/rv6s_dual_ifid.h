#pragma once

#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_ifid.h"

#include "rv6s_dual_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class IFID_DUAL : public IFID {
public:
    IFID_DUAL(std::string name, SimComponent* parent) : IFID(name, parent) {
        CONNECT_REGISTERED_CLEN_INPUT(instr2, clear, enable);
    }

    REGISTERED_CLEN_INPUT(instr2, RV_REG_WIDTH);
};

}  // namespace core
}  // namespace vsrtl
