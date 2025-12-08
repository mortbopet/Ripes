#pragma once

#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_memory.h"
#include "VSRTL/core/vsrtl_wire.h"

#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN, bool readBypass, unsigned regCnt=c_RVRegs, bool zeroIsReadOnly=true>
class RegisterFile : public Component {
public:
  static constexpr unsigned s_regBits = vsrtl::ceillog2(regCnt);
  
  SetGraphicsType(ClockedComponent);
  RegisterFile(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    // Writes

    // Disable writes to register 0
    if constexpr (zeroIsReadOnly) {
      wr_en_0->setSensitiveTo(wr_en);
      wr_en_0->out << [this] { return wr_en.uValue() && wr_addr.uValue() != 0; };
      wr_en_0->out >> _wr_mem->wr_en;
    } else {
      wr_en >> _wr_mem->wr_en;
    }

    wr_addr >> _wr_mem->addr;
    data_in >> _wr_mem->data_in;
    (XLEN / CHAR_BIT) >> _wr_mem->wr_width;

    // Reads
    r1_addr >> _rd1_mem->addr;
    r2_addr >> _rd2_mem->addr;

    /** If read bypassing is enabled, we may read the next-state register value
     * in the current state. Also note that, given that the RegisterFile is of
     * type Component, all inputs must be propagated before outputs are
     * propagated. Thus, we are sure to have received the next-state write
     * address when we clock the output ports. This would >not< be the case if
     * the RegisterFile was a clocked component.
     */
    if constexpr (readBypass) {
      r1_out << [this] { return doReadBypass( r1_addr, _rd1_mem ); };
      r2_out << [this] { return doReadBypass( r2_addr, _rd2_mem ); };
    } else {
      _rd1_mem->data_out >> r1_out;
      _rd2_mem->data_out >> r2_out;
    }
  }

  SUBCOMPONENT(_wr_mem, TYPE(WrMemory<s_regBits, XLEN, false>));
  SUBCOMPONENT(_rd1_mem, TYPE(RdMemory<s_regBits, XLEN, false>));
  SUBCOMPONENT(_rd2_mem, TYPE(RdMemory<s_regBits, XLEN, false>));

  INPUTPORT(r1_addr, s_regBits);
  INPUTPORT(r2_addr, s_regBits);
  INPUTPORT(wr_addr, s_regBits);

  INPUTPORT(data_in, XLEN);
  WIRE(wr_en_0, 1); // only used when zeroIsReadOnly == true
  INPUTPORT(wr_en, 1);
  OUTPUTPORT(r1_out, XLEN);
  OUTPUTPORT(r2_out, XLEN);

  VSRTL_VT_U getRegister(unsigned i) const {
    return m_memory->readMemConst(i << ceillog2(XLEN / CHAR_BIT),
                                  XLEN / CHAR_BIT);
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
  VSRTL_VT_U doReadBypass(decltype(r1_addr)& addr, decltype(_rd1_mem)& mem ) const {
    const unsigned rd_idx = addr.uValue();
    if constexpr (zeroIsReadOnly) {
      if (rd_idx == 0) {
        return VT_U(0);
      }
    }

    const unsigned wr_idx = wr_addr.uValue();
    if (wr_en.uValue() && wr_idx == rd_idx) {
      return data_in.uValue();
    } else {
      return mem->data_out.uValue();
    }
  }

  AddressSpace *m_memory = nullptr;
};

template <unsigned XLEN, bool readBypass, unsigned regCnt=c_RVRegs, bool zeroIsReadOnly=true>
class RegisterFile3 : public RegisterFile<XLEN, readBypass, regCnt, zeroIsReadOnly> {
  using RF = RegisterFile<XLEN, readBypass, regCnt, zeroIsReadOnly>;
public:
  RegisterFile3(const std::string &name, SimComponent *parent)
      : RF(name, parent) {
    // Reads
    r3_addr >> _rd3_mem->addr;

    /** If read bypassing is enabled, we may read the next-state register value
     * in the current state. Also note that, given that the RegisterFile is of
     * type Component, all inputs must be propagated before outputs are
     * propagated. Thus, we are sure to have received the next-state write
     * address when we clock the output ports. This would >not< be the case if
     * the RegisterFile was a clocked component.
     */
    if constexpr (readBypass) {
      r3_out << [this] { return doReadBypass( r3_addr, _rd3_mem ); };
    } else {
      _rd3_mem->data_out >> r3_out;
    }
  }

  SUBCOMPONENT(_rd3_mem, TYPE(RdMemory<RF::s_regBits, XLEN, false>));
  INPUTPORT(r3_addr, RF::s_regBits);
  OUTPUTPORT(r3_out, XLEN);
  
  void setMemory(AddressSpace *mem) override {
    RF::setMemory(mem);
    _rd3_mem->setMemory(RF::m_memory);
  }
};

} // namespace core
} // namespace vsrtl
