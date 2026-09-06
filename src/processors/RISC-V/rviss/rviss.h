#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_design.h"

#include "../../ripesvsrtlprocessor.h"

#include "processors/RISC-V/riscv.h"
#include "processors/RISC-V/rv_uncompress.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief The ISASimCore class
 * A single, opaque VSRTL box representing the ISA-simulator core. It carries no
 * datapath - the actual execution is performed by the software interpreter in
 * RVISS::executeInstruction - but gives the processor a graphical
 * representation consistent with the rest of Ripes.
 */
class ISASimCore : public Component {
public:
  ISASimCore(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    setDescription(
        "RISC-V ISA simulator core.\n\nExecutes one full instruction per clock "
        "cycle through a software interpreter (a single-step ISA model), "
        "rather than a cycle-accurate datapath.");
    // A constant status output, so the component has a driven port and renders
    // as a self-contained box.
    running << [] { return 1; };
  }

  OUTPUTPORT(running, 1);
};

/**
 * @brief The RVISS class
 * An "ISA simulator" processor model. Unlike the other Ripes RISC-V processors,
 * which are cycle-accurate VSRTL datapaths, this model is a single VSRTL box
 * whose behavior is defined by a software instruction-set interpreter: each
 * clock cycle fetches, (optionally) decompresses, decodes and executes exactly
 * one instruction via a large switch statement. It complies fully with the
 * Ripes processor interface and therefore integrates with the regular processor
 * selection, memory and register views.
 *
 * Supported ISA: RV32I / RV64I base + M + C extensions.
 */
template <typename XLEN_T>
class RVISS : public RipesVSRTLProcessor {
  static_assert(std::is_same<uint32_t, XLEN_T>::value ||
                    std::is_same<uint64_t, XLEN_T>::value,
                "Only supports 32- and 64-bit variants");
  static constexpr unsigned XLEN = sizeof(XLEN_T) * CHAR_BIT;
  using XLEN_S = typename std::make_signed<XLEN_T>::type;

public:
  RVISS(const QStringList &extensions)
      : RipesVSRTLProcessor("RISC-V ISA Simulator") {
    m_enabledISA = ISAInfoRegistry::getISA<XLenToRVISA<XLEN>()>(extensions);
    m_cEnabled = m_enabledISA->extensionEnabled("C");
    // This is a pure software interpreter: it is not reversible and exposes no
    // cache interface (memory is accessed directly, not through a datapath).
    m_features = 0;
    m_regs.fill(0);
  }

  // The single VSRTL "box".
  SUBCOMPONENT(core, ISASimCore);

  // Main (byte-addressable) memory.
  ADDRESSSPACEMM(m_memory);

  // =========================== Ripes interface ===========================
  const ProcessorStructure &structure() const override { return m_structure; }
  unsigned int getPcForStage(StageIndex) const override { return m_pc; }
  AInt nextFetchedAddress() const override { return m_pc; }
  QString stageName(StageIndex) const override { return "•"; }
  StageInfo stageInfo(StageIndex) const override {
    return StageInfo({m_pc, isExecutableAddress(m_pc), StageInfo::State::None});
  }
  const std::vector<StageIndex> breakpointTriggeringStages() const override {
    return {{0, 0}};
  }

  void setProgramCounter(AInt address) override { m_pc = address & pcMask(); }
  void setPCInitialValue(AInt address) override { m_pcInitial = address; }
  AddressSpaceMM &getMemory() override { return *m_memory; }

  VInt getRegister(const std::string_view &, unsigned i) const override {
    return m_regs[i];
  }
  void setRegister(const std::string_view &, unsigned i, VInt v) override {
    if (i != 0)
      m_regs[i] = static_cast<XLEN_T>(v);
  }

  MemoryAccess dataMemAccess() const override { return m_lastDataAccess; }
  MemoryAccess instrMemAccess() const override { return m_lastInstrAccess; }

  void finalize(FinalizeReason fr) override {
    if (fr == FinalizeReason::exitSyscall)
      m_finishInNextCycle = true;
  }
  bool finished() const override {
    return m_finished || !stageInfo({0, 0}).stage_valid;
  }
  long long getCycleCount() const override { return m_cycles; }

  void resetProcessor() override {
    m_instructionsRetired = 0;
    m_cycles = 0;
    m_finished = false;
    m_finishInNextCycle = false;
    m_lastDataAccess = MemoryAccess();
    m_lastInstrAccess = MemoryAccess();
    m_regs.fill(0);
    // Resets the registered address spaces (reloading the program) and the box
    // component.
    reset();
    m_pc = m_pcInitial;
  }

  static ProcessorISAInfo supportsISA() { return RVISA::supportsISA<XLEN>(); }
  std::shared_ptr<ISAInfoBase> implementsISA() const override {
    return m_enabledISA;
  }
  std::shared_ptr<const ISAInfoBase> fullISA() const override {
    return RVISA::fullISA<XLEN>();
  }
  const std::set<std::string_view> registerFiles() const override {
    return {RVISA::GPR};
  }

protected:
  void clockProcessor() override {
    m_instructionsRetired++;
    // finalize() (via an exit ecall's trap handler) may set m_finishInNextCycle
    // during executeInstruction(); capture the pre-execution value so we finish
    // exactly one cycle later, matching the datapath models.
    const bool finishInThisCycle = m_finishInNextCycle;
    executeInstruction();
    m_cycles++;
    if (finishInThisCycle)
      m_finished = true;
    // The software interpreter does not clock the VSRTL design, so relay the
    // "clocked" notification to the GUI ourselves.
    processorWasClocked.Emit();
  }

private:
  static constexpr AInt pcMask() {
    return XLEN == 64 ? ~AInt(0) : AInt(0xffffffffu);
  }

  static int64_t signExtend(uint64_t value, unsigned bits) {
    const uint64_t m = uint64_t(1) << (bits - 1);
    return static_cast<int64_t>((value ^ m) - m);
  }

  XLEN_T reg(unsigned i) const { return i == 0 ? XLEN_T(0) : m_regs[i]; }
  void setReg(unsigned i, XLEN_T v) {
    if (i != 0)
      m_regs[i] = v;
  }

  // Portable high-half multiplications (MSVC has no __int128).
  static uint64_t mulhu64(uint64_t a, uint64_t b) {
    const uint64_t aLo = uint32_t(a), aHi = a >> 32;
    const uint64_t bLo = uint32_t(b), bHi = b >> 32;
    const uint64_t ll = aLo * bLo;
    const uint64_t lh = aLo * bHi;
    const uint64_t hl = aHi * bLo;
    const uint64_t hh = aHi * bHi;
    const uint64_t cross = (ll >> 32) + uint32_t(lh) + uint32_t(hl);
    return hh + (lh >> 32) + (hl >> 32) + (cross >> 32);
  }
  static int64_t mulh64(int64_t a, int64_t b) {
    uint64_t hi = mulhu64(uint64_t(a), uint64_t(b));
    if (a < 0)
      hi -= uint64_t(b);
    if (b < 0)
      hi -= uint64_t(a);
    return int64_t(hi);
  }
  static int64_t mulhsu64(int64_t a, uint64_t b) {
    uint64_t hi = mulhu64(uint64_t(a), b);
    if (a < 0)
      hi -= b;
    return int64_t(hi);
  }

  // 'M' extension, full XLEN width.
  XLEN_T mExt(unsigned funct3, XLEN_T a, XLEN_T b) const {
    using U = XLEN_T;
    using S = XLEN_S;
    switch (funct3) {
    case 0b000: // MUL
      return U(a * b);
    case 0b001: // MULH (signed x signed, high)
      if constexpr (XLEN == 32)
        return U((int64_t(int32_t(a)) * int64_t(int32_t(b))) >> 32);
      else
        return U(mulh64(int64_t(a), int64_t(b)));
    case 0b010: // MULHSU (signed x unsigned, high)
      if constexpr (XLEN == 32)
        return U((int64_t(int32_t(a)) * int64_t(uint32_t(b))) >> 32);
      else
        return U(mulhsu64(int64_t(a), uint64_t(b)));
    case 0b011: // MULHU (unsigned x unsigned, high)
      if constexpr (XLEN == 32)
        return U((uint64_t(uint32_t(a)) * uint64_t(uint32_t(b))) >> 32);
      else
        return U(mulhu64(uint64_t(a), uint64_t(b)));
    case 0b100: // DIV
      if (b == 0)
        return U(-1);
      if (S(a) == std::numeric_limits<S>::min() && S(b) == S(-1))
        return a;
      return U(S(a) / S(b));
    case 0b101: // DIVU
      if (b == 0)
        return ~U(0);
      return U(a / b);
    case 0b110: // REM
      if (b == 0)
        return a;
      if (S(a) == std::numeric_limits<S>::min() && S(b) == S(-1))
        return 0;
      return U(S(a) % S(b));
    case 0b111: // REMU
      if (b == 0)
        return a;
      return U(a % b);
    }
    return 0;
  }

  // 'M' extension, 32-bit word ops (RV64 '*W' variants).
  static uint32_t mExtW(unsigned funct3, uint32_t a, uint32_t b) {
    switch (funct3) {
    case 0b000: // MULW
      return a * b;
    case 0b100: // DIVW
      if (b == 0)
        return uint32_t(-1);
      if (int32_t(a) == std::numeric_limits<int32_t>::min() && int32_t(b) == -1)
        return a;
      return uint32_t(int32_t(a) / int32_t(b));
    case 0b101: // DIVUW
      if (b == 0)
        return ~uint32_t(0);
      return a / b;
    case 0b110: // REMW
      if (b == 0)
        return a;
      if (int32_t(a) == std::numeric_limits<int32_t>::min() && int32_t(b) == -1)
        return 0;
      return uint32_t(int32_t(a) % int32_t(b));
    case 0b111: // REMUW
      if (b == 0)
        return a;
      return a % b;
    }
    return 0;
  }

  void executeInstruction();

  std::shared_ptr<ISAInfoBase> m_enabledISA;
  bool m_cEnabled = false;
  ProcessorStructure m_structure = {{0, 1}};

  std::array<XLEN_T, 32> m_regs{};
  AInt m_pc = 0;
  AInt m_pcInitial = 0;
  long long m_cycles = 0;
  bool m_finished = false;
  bool m_finishInNextCycle = false;
  MemoryAccess m_lastDataAccess;
  MemoryAccess m_lastInstrAccess;
};

template <typename XLEN_T>
void RVISS<XLEN_T>::executeInstruction() {
  constexpr bool rv64 = (XLEN == 64);
  const unsigned shamtMask = rv64 ? 0x3f : 0x1f;
  const AInt pc = m_pc;

  // ---- Fetch + (optional) 'C' decompression -----------------------------
  const uint32_t raw = static_cast<uint32_t>(m_memory->readMem(pc, 4));
  bool pcInc4 = true;
  const uint32_t instr = static_cast<uint32_t>(
      uncompressInstruction(raw, m_cEnabled, rv64, pcInc4));
  const unsigned instrBytes = pcInc4 ? 4u : 2u;
  const AInt pcNext = (pc + instrBytes) & pcMask();
  m_lastInstrAccess = MemoryAccess{MemoryAccess::Read, pc, instrBytes};
  m_lastDataAccess = MemoryAccess();

  // ---- Decode fields ----------------------------------------------------
  const unsigned opcode = instr & 0x7f;
  const unsigned rd = (instr >> 7) & 0x1f;
  const unsigned funct3 = (instr >> 12) & 0x7;
  const unsigned rs1 = (instr >> 15) & 0x1f;
  const unsigned rs2 = (instr >> 20) & 0x1f;
  const unsigned funct7 = (instr >> 25) & 0x7f;

  const int64_t immI = signExtend(instr >> 20, 12);
  const int64_t immS =
      signExtend(((instr >> 25) << 5) | ((instr >> 7) & 0x1f), 12);
  const int64_t immB =
      signExtend((((instr >> 31) & 1) << 12) | (((instr >> 25) & 0x3f) << 5) |
                     (((instr >> 8) & 0xf) << 1) | (((instr >> 7) & 1) << 11),
                 13);
  const int64_t immU = static_cast<int32_t>(instr & 0xfffff000u);
  const int64_t immJ = signExtend(
      (((instr >> 31) & 1) << 20) | (((instr >> 21) & 0x3ff) << 1) |
          (((instr >> 20) & 1) << 11) | (((instr >> 12) & 0xff) << 12),
      21);

  AInt nextPC = pcNext;

  // ---- Execute ----------------------------------------------------------
  switch (opcode) {
  case RVISA::LUI:
    setReg(rd, static_cast<XLEN_T>(immU));
    break;
  case RVISA::AUIPC:
    setReg(rd, static_cast<XLEN_T>(pc) + static_cast<XLEN_T>(immU));
    break;
  case RVISA::JAL:
    setReg(rd, static_cast<XLEN_T>(pcNext));
    nextPC = (pc + static_cast<AInt>(immJ)) & pcMask();
    break;
  case RVISA::JALR: {
    const AInt target =
        (static_cast<AInt>(reg(rs1)) + static_cast<AInt>(immI)) & ~AInt(1);
    setReg(rd, static_cast<XLEN_T>(pcNext));
    nextPC = target & pcMask();
    break;
  }
  case RVISA::BRANCH: {
    const XLEN_T a = reg(rs1), b = reg(rs2);
    bool taken = false;
    switch (funct3) {
    case 0b000:
      taken = a == b;
      break; // BEQ
    case 0b001:
      taken = a != b;
      break; // BNE
    case 0b100:
      taken = XLEN_S(a) < XLEN_S(b);
      break; // BLT
    case 0b101:
      taken = XLEN_S(a) >= XLEN_S(b);
      break; // BGE
    case 0b110:
      taken = a < b;
      break; // BLTU
    case 0b111:
      taken = a >= b;
      break; // BGEU
    default:
      break;
    }
    if (taken)
      nextPC = (pc + static_cast<AInt>(immB)) & pcMask();
    break;
  }
  case RVISA::LOAD: {
    const AInt addr =
        (static_cast<AInt>(reg(rs1)) + static_cast<AInt>(immI)) & pcMask();
    unsigned bytes = 0;
    bool sign = false;
    switch (funct3) {
    case 0b000:
      bytes = 1;
      sign = true;
      break; // LB
    case 0b001:
      bytes = 2;
      sign = true;
      break; // LH
    case 0b010:
      bytes = 4;
      sign = true;
      break; // LW
    case 0b011:
      bytes = 8;
      sign = true;
      break; // LD
    case 0b100:
      bytes = 1;
      break; // LBU
    case 0b101:
      bytes = 2;
      break; // LHU
    case 0b110:
      bytes = 4;
      break; // LWU
    default:
      break;
    }
    if (bytes) {
      const uint64_t val = m_memory->readMem(addr, bytes);
      const XLEN_T result =
          sign ? static_cast<XLEN_T>(signExtend(val, bytes * 8))
               : static_cast<XLEN_T>(val);
      setReg(rd, result);
      m_lastDataAccess = MemoryAccess{MemoryAccess::Read, addr, bytes};
    }
    break;
  }
  case RVISA::STORE: {
    const AInt addr =
        (static_cast<AInt>(reg(rs1)) + static_cast<AInt>(immS)) & pcMask();
    unsigned bytes = 0;
    switch (funct3) {
    case 0b000:
      bytes = 1;
      break; // SB
    case 0b001:
      bytes = 2;
      break; // SH
    case 0b010:
      bytes = 4;
      break; // SW
    case 0b011:
      bytes = 8;
      break; // SD
    default:
      break;
    }
    if (bytes) {
      m_memory->writeMem(addr, reg(rs2), bytes);
      m_lastDataAccess = MemoryAccess{MemoryAccess::Write, addr, bytes};
    }
    break;
  }
  case RVISA::OPIMM: {
    const XLEN_T a = reg(rs1);
    const XLEN_T imm = static_cast<XLEN_T>(immI);
    XLEN_T res = 0;
    switch (funct3) {
    case 0b000:
      res = a + imm;
      break; // ADDI
    case 0b010:
      res = (XLEN_S(a) < XLEN_S(imm)) ? 1 : 0;
      break; // SLTI
    case 0b011:
      res = (a < imm) ? 1 : 0;
      break; // SLTIU
    case 0b100:
      res = a ^ imm;
      break; // XORI
    case 0b110:
      res = a | imm;
      break; // ORI
    case 0b111:
      res = a & imm;
      break; // ANDI
    case 0b001:
      res = a << ((instr >> 20) & shamtMask);
      break; // SLLI
    case 0b101: {
      const unsigned sh = (instr >> 20) & shamtMask;
      res = ((instr >> 30) & 1) ? static_cast<XLEN_T>(XLEN_S(a) >> sh) // SRAI
                                : (a >> sh);                           // SRLI
      break;
    }
    default:
      break;
    }
    setReg(rd, res);
    break;
  }
  case RVISA::OP: {
    const XLEN_T a = reg(rs1), b = reg(rs2);
    XLEN_T res = 0;
    if (funct7 == 0b0000001) {
      res = mExt(funct3, a, b); // 'M' extension
    } else {
      switch (funct3) {
      case 0b000:
        res = (funct7 == 0b0100000) ? (a - b) : (a + b);
        break; // ADD / SUB
      case 0b001:
        res = a << (b & shamtMask);
        break; // SLL
      case 0b010:
        res = (XLEN_S(a) < XLEN_S(b)) ? 1 : 0;
        break; // SLT
      case 0b011:
        res = (a < b) ? 1 : 0;
        break; // SLTU
      case 0b100:
        res = a ^ b;
        break; // XOR
      case 0b101:
        res = (funct7 == 0b0100000)
                  ? static_cast<XLEN_T>(XLEN_S(a) >> (b & shamtMask)) // SRA
                  : (a >> (b & shamtMask));                           // SRL
        break;
      case 0b110:
        res = a | b;
        break; // OR
      case 0b111:
        res = a & b;
        break; // AND
      default:
        break;
      }
    }
    setReg(rd, res);
    break;
  }
  case RVISA::OPIMM32: { // RV64 word immediate ops
    if (rv64) {
      const uint32_t a = static_cast<uint32_t>(reg(rs1));
      uint32_t res = 0;
      switch (funct3) {
      case 0b000:
        res = a + static_cast<uint32_t>(immI);
        break; // ADDIW
      case 0b001:
        res = a << ((instr >> 20) & 0x1f);
        break; // SLLIW
      case 0b101: {
        const unsigned sh = (instr >> 20) & 0x1f;
        res = ((instr >> 30) & 1) ? static_cast<uint32_t>(int32_t(a) >> sh)
                                  : (a >> sh); // SRAIW / SRLIW
        break;
      }
      default:
        break;
      }
      setReg(rd, static_cast<XLEN_T>(static_cast<int64_t>(int32_t(res))));
    }
    break;
  }
  case RVISA::OP32: { // RV64 word register ops
    if (rv64) {
      const uint32_t a = static_cast<uint32_t>(reg(rs1));
      const uint32_t b = static_cast<uint32_t>(reg(rs2));
      uint32_t res = 0;
      if (funct7 == 0b0000001) {
        res = mExtW(funct3, a, b); // 'M' extension words
      } else {
        switch (funct3) {
        case 0b000:
          res = (funct7 == 0b0100000) ? (a - b) : (a + b);
          break; // ADDW / SUBW
        case 0b001:
          res = a << (b & 0x1f);
          break; // SLLW
        case 0b101:
          res = (funct7 == 0b0100000)
                    ? static_cast<uint32_t>(int32_t(a) >> (b & 0x1f)) // SRAW
                    : (a >> (b & 0x1f));                              // SRLW
          break;
        default:
          break;
        }
      }
      setReg(rd, static_cast<XLEN_T>(static_cast<int64_t>(int32_t(res))));
    }
    break;
  }
  case RVISA::SYSTEM: {
    // ECALL (funct3 == 0, imm == 0). Other SYSTEM instructions (CSR access,
    // EBREAK) are treated as no-ops by this model.
    if (funct3 == 0 && ((instr >> 20) & 0xfff) == 0) {
      if (trapHandler)
        trapHandler();
    }
    break;
  }
  default:
    break;
  }

  m_pc = nextPC;
}

} // namespace core
} // namespace vsrtl
