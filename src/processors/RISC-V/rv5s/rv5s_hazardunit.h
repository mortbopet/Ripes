#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class HazardUnit : public Component {
public:
    HazardUnit(std::string name, SimComponent* parent) : Component(name, parent) {}
};
}  // namespace core
}  // namespace vsrtl
