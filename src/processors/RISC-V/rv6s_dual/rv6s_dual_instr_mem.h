#pragma once

#include "VSRTL/core/vsrtl_memory.h"

namespace vsrtl {
namespace core {

/**
 * @brief ROM_DUAL
 * Specialization of a rom, which outputs mem[addr] and mem[addr + addrWidth] on each cycle.
 */
template <unsigned int addrWidth, unsigned int dataWidth, bool byteIndexed = true>
class ROM_DUAL : public ROM<addrWidth, dataWidth, byteIndexed> {
public:
    ROM_DUAL(std::string name, SimComponent* parent) : ROM<addrWidth, dataWidth, byteIndexed>(name, parent) {
        data_out2 << [=] {
            const auto accessAddress = this->addr.uValue() + 4;
            const auto v = this->read(accessAddress);
            return v;
        };
    }

    OUTPUTPORT(data_out2, dataWidth);
};

}  // namespace core
}  // namespace vsrtl
