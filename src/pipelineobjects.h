#ifndef PIPELINEOBJECTS_H
#define PIPELINEOBJECTS_H

#include "binutils.h"
#include "defines.h"
#include "mainmemory.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

/* PIPELINE OBJECTS
 * all pipeline objects are built around the Signal class - a boolean vector of immutable size.
 * The Signal class ensures size compatability of I/O signals, much like regular HDL languages
 * Signals can be cast to signed/unsigned integers or boolean values, to easily use them in ie. computational expression
 * or logic expressions
 *
 * All combinatorial components have an update() function, which corresponds to propagating the
 * input signal values through the combinatorial circuit
 *
 * Registers have a clock() function, which sets the output value to the input value
 */

// Signal class
// A boolean vector of immutable size
// Can be cast to booleans, u/integers etc.
#define ASSERT_SIZE static_assert(n >= 1 && n <= 64, "n = [1;64]");
#define SETNAME m_name = name;

class SignalBase {
public:
    uint32_t getValue() const { return m_value; }

protected:
    uint32_t m_value = 0;
    std::string m_name;
};

template <int n>
class Signal : public SignalBase {
public:
    // Constructors
    Signal(std::string name = ""){ASSERT_SIZE SETNAME};
    Signal(std::vector<bool> v, std::string name = "") {
        ASSERT_SIZE
        // Builds a signal from a std::vector
        SETNAME
        if (v.size() != n) {
            throw std::invalid_argument("Input vector size does not match Signal size");
        }
        m_value = v;
    }

    // Mark as explicit, so new signals are not constructed upon assignment to simple types
    explicit Signal(uint32_t v, std::string name = "") {
        ASSERT_SIZE
        SETNAME
        m_value = v;
    }
    explicit Signal(int v, std::string name = "") {
        ASSERT_SIZE
        SETNAME
        m_value = signextend<int32_t, n>(v);
    }

    void setName(std::string name) { m_name = name; }

    // Casting operators
    explicit operator int() const { return signextend<int32_t, n>(m_value); }
    explicit operator uint32_t() const { return m_value; }
    explicit operator uint64_t() const { return m_value; }
    explicit operator int64_t() const { return signextend<int64_t, n>(m_value); }
    explicit operator bool() const { return m_value & 0b1; }

    Signal& operator=(const uint32_t& v) {
        m_value = v;
        return *this;
    }
    Signal& operator=(const int& v) {
        *this = static_cast<uint32_t>(v);
        return *this;
    }
    Signal& operator=(const Signal& other) {
        m_value = other.getValue();
        return *this;
    }
};

// The Reg class is used for sequential signal assignment. Has a single input/output of a given signal size.
// Upon construction, new registers are added to "registers", which is used when clocking the pipeline
// Input "Reset" is a synchronous reset
// input "Enable" is a synchronous clock enable signal. If not set (nullptr), it is tied to High (always enabled)
class RegBase {
    friend class RegBank;

public:
    static std::vector<RegBase*> registers;
    static void clockAll() {
        // Each registers input value is save before clocking it, to ensure that register -> register connections are
        // clocked properly
        std::for_each(registers.begin(), registers.end(), [](auto& reg) { reg->save(); });
        std::for_each(registers.begin(), registers.end(), [](auto& reg) { reg->clock(); });
    }

    static void resetAll() {
        std::for_each(registers.begin(), registers.end(), [](auto& reg) { reg->reset(); });
    }
    void setReset(const Signal<1>* in) { m_reset = in; }
    void setEnable(const Signal<1>* in) { m_enable = in; }

protected:
    virtual void clock() = 0;
    virtual void save() = 0;
    virtual void reset() = 0;

    const Signal<1>* m_reset;
    const Signal<1>* m_enable;
};

class RegBank {
public:
    // Collection of RegBase pointers - for grouping registers and setting equal control signals to all
    RegBank& addToBank(RegBase* regPtr) {
        registers.push_back(regPtr);
        return *this;
    }
    void setReset(const Signal<1>* sig) { m_reset = sig; }
    void setEnable(const Signal<1>* sig) { m_enable = sig; }
    void setRegisterControls() {
        if (m_reset != nullptr)
            std::for_each(registers.begin(), registers.end(), [this](auto& reg) { reg->setReset(m_reset); });
        if (m_enable != nullptr)
            std::for_each(registers.begin(), registers.end(), [this](auto& reg) { reg->setEnable(m_enable); });
    }

private:
    std::vector<RegBase*> registers;
    const Signal<1>* m_reset;
    const Signal<1>* m_enable;
};

template <int n>
class Reg : public RegBase {
public:
    Reg() {
        ASSERT_SIZE
        registers.push_back(this);
    }

    // Signal connection
    void connect(Reg<n>& r) { setInput(&r.m_current); }
    void connect(const Signal<n>* s) { setInput(s); }
    Signal<n>* getOutput() { return &m_current; }
    void overrideNext(uint32_t val) {
        // Only used when transforming invalidPC reasons for a stage (for proper GUI visualization)
        // sets the next state value of m_current to the overriden value, regardles of m_next
        // WARNING! This disregards all control signals (clock enable, reset) etc.
        m_overrideNext = true;
        m_nextSaved = val;
    }

    explicit operator int() const { return (int)m_current; }
    explicit operator uint32_t() const { return (uint32_t)m_current; }
    explicit operator bool() const { return (bool)m_current; }
    void setInput(const Signal<n>* in) { m_next = in; }

protected:
    void clock() override {
        if (m_enable == nullptr || (bool)*m_enable == true) {
            if (m_reset == nullptr)
                m_current = m_nextSaved;
            else if ((bool)*m_reset == true) {
                m_current = 0;
            } else {
                m_current = m_nextSaved;
            }
        } else {
            // Do nothing - clock enable is deasserted
        }
        if (m_overrideNext) {
            // disregard control signals and override current value
            m_current = m_nextSaved;
            m_overrideNext = false;
        }
    }
    void save() override {
        assert(m_next != nullptr);
        if (!m_overrideNext) {
            m_nextSaved = *m_next;
        }
    }
    void reset() override {
        m_current = Signal<n>(0);
        m_nextSaved = Signal<n>(0);
    }

private:
    Signal<n> m_current;
    Signal<n> m_nextSaved;
    const Signal<n>* m_next;
    bool m_overrideNext = false;
};

template <int inputs, int n>
class Combinational {
public:
    Combinational() { static_assert(n >= 1 && n <= 32 && inputs > 0, "Invalid multiplexer size specification"); }

    virtual void update() = 0;
    bool setInput(int input, const Signal<n>* sig) {
        if (input < 0 || input >= inputs)
            return false;
        m_inputs[input] = sig;
        return true;
    }

    void setControl(const Signal<bitcount(inputs)>* sig) { m_control = sig; }

    Signal<n>* getOutput() { return &m_output; }
    explicit operator uint32_t() const { return (uint32_t)m_output; }
    explicit operator int() const { return (int)m_output; }

protected:
    std::vector<const Signal<n>*> m_inputs = std::vector<const Signal<n>*>(inputs);
    const Signal<bitcount(inputs)>* m_control;  // control size = roof(log2(inputs))
    Signal<n> m_output;

    bool initialized() {
        bool b = true;
        b &= m_control != nullptr;
        for (const auto& input : m_inputs) {
            b &= input != nullptr;
        }
        return b;
    }
};

// Multiplexor class
// the output of the multiplexor is dependant of the input select signal
template <int inputs, int n>
class Mux : public Combinational<inputs, n> {
public:
    void update() override {
        {
            if (!this->initialized())
                throw std::runtime_error("Mux not initialized");
            this->m_output = (uint32_t) * this->m_inputs[(uint32_t) * this->m_control];
        }
    }
};
enum class GateType { AND, OR, XOR };
template <int inputs, int n, GateType type>
class Gate : public Combinational<inputs, n> {
    // Sets an operand functor according to the input GateType, which is used in computing the output of the gate
    // Currently, only 1-bit evaluation is supported
public:
    Gate() {
        switch (type) {
            case GateType::AND:
                m_op = [](bool a, const Signal<n>& b) { return a & (bool)b; };
                break;
            case GateType::OR:
                m_op = [](bool a, const Signal<n>& b) { return a | (bool)b; };
                break;
            case GateType::XOR:
                m_op = [](bool a, const Signal<n>& b) { return a ^ (bool)b; };
                break;
        }
    }
    void update() override {
        bool b = true;
        for (const auto& input : this->m_inputs) {
            b = m_op(b, *input);
        }
        this->m_output = Signal<n>(b);
    }

private:
    std::function<bool(bool, const Signal<n>&)> m_op;
};

namespace ALUOps {
static const int CTRL_SIZE = 5;
enum OPCODE {
    ADD = 0,
    SUB = 1,
    MUL = 2,
    DIV = 3,
    AND = 4,
    OR = 5,
    XOR = 6,
    SL = 7,
    SRA = 8,
    SRL = 9,
    LUI = 10,
    LT = 11,
    LTU = 12,
    EQ = 13,
    MULH = 14,
    MULHU = 15,
    MULHSU = 16,
    DIVU = 17,
    REM = 18,
    REMU = 19
};
}  // namespace ALUOps

template <int n>
class ALU {
public:
    // When calculating ALU control signals, the OPCODE is implicitely converted to an int in the Signal, which is
    // later reinterpreted as an OPCODE
    ALU(std::string name = "ALU") { m_name = name; }
    void update();

    void setInputs(Signal<n>* s1, Signal<n>* s2) {
        m_op1 = s1;
        m_op2 = s2;
    }

    Signal<n>* getOutput() { return &m_output; }

    void setControl(const Signal<ALUOps::CTRL_SIZE>* sig) { m_control = sig; }

private:
    bool initialized() {
        bool b = true;
        b &= m_control != nullptr;
        b &= m_op1 != nullptr;
        b &= m_op2 != nullptr;
        return b;
    }
    std::string m_name;
    Signal<n> m_output;
    const Signal<n>* m_op1;
    const Signal<n>* m_op2;
    const Signal<ALUOps::CTRL_SIZE>* m_control;
};

template <int n>
void ALU<n>::update() {
    if (!initialized())
        throw std::runtime_error("Mux not initialized");
    switch ((ALUOps::OPCODE)(uint32_t)*m_control) {
        case ALUOps::ADD:
            m_output = (uint32_t)*m_op1 + (uint32_t)*m_op2;
            break;
        case ALUOps::SUB:
            m_output = (uint32_t)*m_op1 - (uint32_t)*m_op2;
            break;
        case ALUOps::MUL:
            m_output = (int)*m_op1 * (int)*m_op2;
            break;
        case ALUOps::MULH: {
            int64_t res = static_cast<int64_t>(*m_op1) * static_cast<int64_t>(*m_op2);
            m_output = static_cast<uint32_t>(res >> 32);
            break;
        }
        case ALUOps::MULHU: {
            int64_t res = static_cast<uint64_t>(*m_op1) * static_cast<uint64_t>(*m_op2);
            m_output = static_cast<uint32_t>(res >> 32);
            break;
        }
        case ALUOps::MULHSU: {
            int64_t res = static_cast<int64_t>(*m_op1) * static_cast<uint64_t>(*m_op2);
            m_output = static_cast<uint32_t>(res >> 32);
            break;
        }
        case ALUOps::DIV:
            if ((int)*m_op2 == 0) {
                m_output = -1;
            } else if ((int)*m_op1 == (-(std::pow(2, 32 - 1))) && (int)*m_op2 == -1) {
                // Overflow
                m_output = static_cast<int>(-(std::pow(2, 32 - 1)));
            } else {
                m_output = (int)*m_op1 / (int)*m_op2;
            }
            break;
        case ALUOps::DIVU:
            if ((int)*m_op2 == 0) {
                m_output = static_cast<uint32_t>(std::pow(2, 32) - 1);
            } else {
                m_output = (uint32_t)*m_op1 / (uint32_t)*m_op2;
            }
            break;
        case ALUOps::REM:
            if ((int)*m_op2 == 0) {
                m_output = (int)*m_op1;
            } else if ((int)*m_op1 == (-(std::pow(2, 32 - 1))) && (int)*m_op2 == -1) {
                // Overflow
                m_output = 0;
            } else {
                m_output = (int)*m_op1 % (int)*m_op2;
            }
            break;
        case ALUOps::REMU:
            if ((uint32_t)*m_op2 == 0) {
                m_output = (uint32_t)*m_op1;
            } else {
                m_output = (uint32_t)*m_op1 % (uint32_t)*m_op2;
            }
            break;
        case ALUOps::AND:
            m_output = (uint32_t)*m_op1 & (uint32_t)*m_op2;
            break;
        case ALUOps::OR:
            m_output = (uint32_t)*m_op1 | (uint32_t)*m_op2;
            break;
        case ALUOps::XOR:
            m_output = (uint32_t)*m_op1 ^ (uint32_t)*m_op2;
            break;
        case ALUOps::SL:
            m_output = (uint32_t)*m_op1 << (uint32_t)*m_op2;
            break;
        case ALUOps::SRA:
            m_output = (int32_t)*m_op1 >> (uint32_t)*m_op2;
            break;
        case ALUOps::SRL:
            m_output = (uint32_t)*m_op1 >> (uint32_t)*m_op2;
            break;
        case ALUOps::LUI:
            m_output = (uint32_t)*m_op2;
            break;
        case ALUOps::LT:
            m_output = (int)*m_op1 < (int)*m_op2 ? 1 : 0;
            break;
        case ALUOps::LTU:
            m_output = (uint32_t)*m_op1 < (uint32_t)*m_op2 ? 1 : 0;
            break;
        default:
            throw std::runtime_error("Invalid ALU opcode");
            break;
    }
}

class Registers {
public:
    Registers() {}

    std::vector<uint32_t>* getRegPtr() { return &m_reg; }
    void update();
    void clock();
    void clear() {
        for (auto& reg : m_reg)
            reg = 0;
    }
    void init();

    void setInputs(Signal<5>* readRegister1, Signal<5>* readRegister2, Signal<5>* writeReg, Signal<32>* writeData,
                   Signal<1>* regWrite);
    Signal<32>* getOutput(int n = 1) { return n == 2 ? &m_readData2 : &m_readData1; }

    // Direct accessors for a0 & a1 registers (ECALL functionality)
    uint32_t a0() const { return m_reg[10]; }
    uint32_t a1() const { return m_reg[11]; }

private:
    // ReadRegister 1/2 is deduced from the input instruction signal
    const Signal<1>* m_regWrite;
    const Signal<5>* m_readRegister1;
    const Signal<5>* m_readRegister2;
    const Signal<5>* m_writeRegister;
    const Signal<32>* m_writeData;
    Signal<32> m_readData1;
    Signal<32> m_readData2;

    std::vector<uint32_t> m_reg = std::vector<uint32_t>(32, 0);  // Internal registers

    std::string m_name = std::string("Registers");
};

#endif  // PIPELINEOBJECTS_H
