#pragma once

#include "processors/RISC-V/riscv.h"

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class BranchSimple : public Component {
public:
  BranchSimple(std::string name, SimComponent *parent)
      : Component(name, parent) {
    comp_op >> multplexer->select;

    sign >> *not_sign->in[0];
    zero >> *not_zero->in[0];
    carry >> *not_carry->in[0];

    not_sign->out >> *bge_or->in[0];
    zero >> *bge_or->in[1];

    0 >> multplexer->get(CompOp::NOP);
    zero >> multplexer->get(CompOp::EQ);
    not_zero->out >> multplexer->get(CompOp::NE);
    sign >> multplexer->get(CompOp::LT);
    bge_or->out >> multplexer->get(CompOp::GE);
    carry >> multplexer->get(CompOp::LTU);
    not_carry->out >> multplexer->get(CompOp::GEU);

    multplexer->out >> res;
  }

  SUBCOMPONENT(multplexer, TYPE(EnumMultiplexer<CompOp, 1>));

  SUBCOMPONENT(bge_or, TYPE(Or<1, 2>));
  SUBCOMPONENT(not_zero, TYPE(Not<1, 1>));
  SUBCOMPONENT(not_sign, TYPE(Not<1, 1>));
  SUBCOMPONENT(not_carry, TYPE(Not<1, 1>));

  INPUTPORT_ENUM(comp_op, CompOp);
  INPUTPORT(zero, 1);
  INPUTPORT(sign, 1);
  INPUTPORT(carry, 1);
  OUTPUTPORT(res, 1);
};

} // namespace core
} // namespace vsrtl
