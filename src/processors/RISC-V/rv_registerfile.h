#pragma once

#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_memory.h"
#include "VSRTL/core/vsrtl_wire.h"
#include "rv_portmatcher.h"

#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned BUS_XLEN, unsigned MEM_XLEN = BUS_XLEN,
          bool readBypass = true, unsigned regCnt = c_RVRegs,
          bool zeroIsReadOnly = true>
class RegisterFile : public Component {
public:
  static constexpr unsigned s_regBits = vsrtl::ceillog2(regCnt);

  SetGraphicsType(ClockedComponent);
  RegisterFile(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    // in port matching
    data_in >> _matcher_data->in;

    // Writes
    // Disable writes to register 0
    wr_en_0->setSensitiveTo(wr_en);
    wr_en_0->out << [this] {
      if constexpr (zeroIsReadOnly) {
        return wr_en.uValue() && wr_addr.uValue() != 0;
      } else {
        return wr_en.uValue();
      }
    };
    wr_en_0->out >> _wr_mem->wr_en;

    wr_addr >> _wr_mem->addr;
    _matcher_data->out >> _wr_mem->data_in;
    (MEM_XLEN / CHAR_BIT) >> _wr_mem->wr_width;

    // Reads
    r1_addr >> _rd1_mem->addr;
    r2_addr >> _rd2_mem->addr;

    _matcher_r1->setSensitiveTo(r1_addr);
    _matcher_r2->setSensitiveTo(r2_addr);
    _matcher_r1->setSensitiveTo(_rd1_mem->data_out);
    _matcher_r2->setSensitiveTo(_rd2_mem->data_out);

    /** If read bypassing is enabled, we may read the next-state register value
     * in the current state. Also note that, given that the RegisterFile is of
     * type Component, all inputs must be propagated before outputs are
     * propagated. Thus, we are sure to have received the next-state write
     * address when we clock the output ports. This would >not< be the case if
     * the RegisterFile was a clocked component.
     */
    if constexpr (readBypass) {
      _matcher_r1->in << [this] { return doReadBypass(r1_addr, _rd1_mem); };
      _matcher_r2->in << [this] { return doReadBypass(r2_addr, _rd2_mem); };
    } else {
      _rd1_mem->data_out >> _matcher_r1->in;
      _rd2_mem->data_out >> _matcher_r2->in;
    }
    // out port matching
    _matcher_r1->out >> r1_out;
    _matcher_r2->out >> r2_out;
  }

  SUBCOMPONENT(_wr_mem, TYPE(WrMemory<s_regBits, MEM_XLEN, false>));
  SUBCOMPONENT(_rd1_mem, TYPE(RdMemory<s_regBits, MEM_XLEN, false>));
  SUBCOMPONENT(_rd2_mem, TYPE(RdMemory<s_regBits, MEM_XLEN, false>));

  INPUTPORT(r1_addr, s_regBits);
  INPUTPORT(r2_addr, s_regBits);
  INPUTPORT(wr_addr, s_regBits);

  INPUTPORT(data_in, BUS_XLEN);
  WIRE(wr_en_0, 1);
  INPUTPORT(wr_en, 1);
  OUTPUTPORT(r1_out, BUS_XLEN);
  OUTPUTPORT(r2_out, BUS_XLEN);

  SUBCOMPONENT(_matcher_data, TYPE(Ported_Matcher<BUS_XLEN, MEM_XLEN>));
  SUBCOMPONENT(_matcher_r1, TYPE(Functioned_Matcher<MEM_XLEN, BUS_XLEN>));
  SUBCOMPONENT(_matcher_r2, TYPE(Functioned_Matcher<MEM_XLEN, BUS_XLEN>));

  VSRTL_VT_U getRegister(unsigned i) const {
    return m_memory->readMemConst(i << ceillog2(MEM_XLEN / CHAR_BIT),
                                  MEM_XLEN / CHAR_BIT);
  }

  std::vector<VSRTL_VT_U> getRegisters() {
    std::vector<VSRTL_VT_U> regs;
    for (int i = 0; i < regCnt; ++i)
      regs.push_back(getRegister(i));
    return regs;
  }

  virtual void setMemory(AddressSpace *mem) {
    m_memory = mem;
    // All memory components must point to the same memory
    _wr_mem->setMemory(m_memory);
    _rd1_mem->setMemory(m_memory);
    _rd2_mem->setMemory(m_memory);
  }

protected:
  VSRTL_VT_U doReadBypass(decltype(r1_addr) &addr,
                          decltype(_rd1_mem) &mem) const {
    const unsigned rd_idx = addr.uValue();
    if constexpr (zeroIsReadOnly) {
      if (rd_idx == 0) {
        return VT_U(0);
      }
    }

    const unsigned wr_idx = wr_addr.uValue();
    if (wr_en.uValue() && wr_idx == rd_idx) {
      return _matcher_data->out.uValue();
    } else {
      return mem->data_out.uValue();
    }
  }

  AddressSpace *m_memory = nullptr;
};

template <unsigned BUS_XLEN, unsigned MEM_XLEN = BUS_XLEN,
          bool readBypass = true, unsigned regCnt = c_RVRegs,
          bool zeroIsReadOnly = true>
class RegisterFile3 : public RegisterFile<BUS_XLEN, MEM_XLEN, readBypass,
                                          regCnt, zeroIsReadOnly> {
  using RF =
      RegisterFile<BUS_XLEN, MEM_XLEN, readBypass, regCnt, zeroIsReadOnly>;

public:
  RegisterFile3(const std::string &name, SimComponent *parent)
      : RF(name, parent) {
    // Reads
    r3_addr >> _rd3_mem->addr;

    _matcher_r3->setSensitiveTo(r3_addr);
    _matcher_r3->setSensitiveTo(_rd3_mem->data_out);

    /** If read bypassing is enabled, we may read the next-state register value
     * in the current state. Also note that, given that the RegisterFile is of
     * type Component, all inputs must be propagated before outputs are
     * propagated. Thus, we are sure to have received the next-state write
     * address when we clock the output ports. This would >not< be the case if
     * the RegisterFile was a clocked component.
     */
    if constexpr (readBypass) {
      _matcher_r3->in <<
          [this] { return this->doReadBypass(r3_addr, _rd3_mem); };
    } else {
      _rd3_mem->data_out >> _matcher_r3->in;
    }

    _matcher_r3->out >> r3_out;
  }

  SUBCOMPONENT(_rd3_mem, TYPE(RdMemory<RF::s_regBits, MEM_XLEN, false>));
  INPUTPORT(r3_addr, RF::s_regBits);
  OUTPUTPORT(r3_out, BUS_XLEN);

  SUBCOMPONENT(_matcher_r3, TYPE(Functioned_Matcher<MEM_XLEN, BUS_XLEN>));

  void setMemory(AddressSpace *mem) override {
    RF::setMemory(mem);
    _rd3_mem->setMemory(RF::m_memory);
  }
};

} // namespace core
} // namespace vsrtl
