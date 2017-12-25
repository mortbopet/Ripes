#ifndef PIPELINEOBJECTS_H
#define PIPELINEOBJECTS_H

#include "binutils.h"
#include "defines.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

/* PIPELINE OBJECTS
 * all pipeline objects are built around the Signal class - a boolean vector of immutable size.
 * The Signal class ensures size compatability of I/O signals, much like regular HDL languages
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
#define CREATE_VEC m_value = std::vector<bool>(n);

template <int n>
class Signal {
public:
    // Constructors
    Signal(){ASSERT_SIZE CREATE_VEC};
    Signal(std::vector<bool> v) {
        ASSERT_SIZE
        // Builds a signal from a std::vector
        if (v.size() != n) {
            throw std::invalid_argument("Input vector size does not match Signal size");
        }
        m_value = v;
    }
    Signal(uint32_t v) {
        ASSERT_SIZE
        CREATE_VEC
        buildVec(m_value, v);
    }
    Signal(int v) {
        ASSERT_SIZE
        CREATE_VEC
        buildVec(m_value, (uint32_t)v);
    }

    // Casting operators
    explicit operator int() const { return signextend<int32_t, n>(accBVec(m_value)); }
    explicit operator uint32_t() const { return accBVec(m_value); }
    explicit operator bool() const { return m_value[0]; }

private:
    std::vector<bool> value() const { return m_value; }
    std::vector<bool> m_value;
};

// Register class
// I/O of registers are handled via. the -> operator
// reg1 -> reg2 connects m_current of reg1 to m_next of reg2

class RegBase {
public:
    virtual void clock() = 0;
};

template <int n>
class Reg : public RegBase {
public:
    Reg() { ASSERT_SIZE }
    void clock() override { m_current = *m_next; }
    uint32_t value() const { return (uint32_t)m_current; }

    // Signal assignment operator
    void connect(Reg<n>& r) { setInput(&r.m_current); }
    void connect(const Signal<n>* s) { setInput(s); }

private:
    void setInput(const Signal<n>* in) { m_next = in; }

    Signal<n> m_current;
    const Signal<n>* m_next;
};

// Multiplexor class
// the output of the multiplexor is dependant of the input select signal
template <int inputs, int n>
class Mux {
public:
    Mux() { static_assert(n >= 1 && n <= 32 && inputs > 0, "Invalid multiplexer size specification"); }

    void update() {
        if (!initialized())
            throw std::runtime_error("Mux not initialized");
        m_output = *m_inputs[*m_control];
    }
    bool setInput(int input, const Signal<n>* sig) {
        if (input < 1 || input >= inputs)
            return false;
        m_inputs[input] = sig;
        return true;
    }
    void setInputs(std::vector<const Signal<n>*> sigs) {
        if (sigs.size() != inputs)
            return false;
        m_inputs = sigs;
    }
    void setControl(std::vector<const Signal<bitcount(inputs)>*> sig) { m_control = sig; }

private:
    std::vector<const Signal<n>*> m_inputs;
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

class ALU {
public:
    // When calculating ALU control signals, the OPCODE is implicitely converted to an int in the Signal, which is later
    // reinterpreted as an OPCODE
    enum OPCODE { ADD, SUB, MUL, DIV, AND, OR, XOR, SL, SRA, SRL, _Count };
    const static int CTRL_SIZE = bitcount(_Count);

    void update();

    void setInputs(std::vector<const Signal<WORDSIZE>*> sigs) {
        if (sigs.size() != 2)
            throw std::invalid_argument("Input vector size must be 2 (2 ALU operands)");
        m_op1 = sigs[0];
        m_op2 = sigs[1];
    }

    void setControl(const Signal<CTRL_SIZE>* sig) { m_control = sig; }

private:
    bool initialized() {
        bool b = true;
        b &= m_control != nullptr;
        b &= m_op1 != nullptr;
        b &= m_op2 != nullptr;
        return b;
    }

    Signal<WORDSIZE> m_output;
    const Signal<WORDSIZE>* m_op1;
    const Signal<WORDSIZE>* m_op2;
    const Signal<CTRL_SIZE>* m_control;
};

void ALU::update() {
    if (!initialized())
        throw std::runtime_error("Mux not initialized");
    switch ((OPCODE)(int)*m_control) {
        case ADD:
            m_output = (uint32_t)*m_op1 + (uint32_t)*m_op2;
            break;
        case SUB:
            m_output = (uint32_t)*m_op1 - (uint32_t)*m_op2;
            break;
        case MUL:
            m_output = (uint32_t)*m_op1 * (uint32_t)*m_op2;
            break;
        case DIV:
            m_output = (uint32_t)*m_op1 / (uint32_t)*m_op2;
            break;
        case AND:
            m_output = (uint32_t)*m_op1 & (uint32_t)*m_op2;
            break;
        case OR:
            m_output = (uint32_t)*m_op1 | (uint32_t)*m_op2;
            break;
        case XOR:
            m_output = (uint32_t)*m_op1 ^ (uint32_t)*m_op2;
            break;
        case SL:
            m_output = (uint32_t)*m_op1 << (uint32_t)*m_op2;
            break;
        case SRA:
            m_output = (uint32_t)*m_op1 >> (uint32_t)*m_op2;
            break;
        case SRL:
            m_output = (int)*m_op1 + (uint32_t)*m_op2;
            break;
        case _Count:
        default:
            throw std::runtime_error("Invalid ALU opcode");
            break;
    }
}
#endif  // PIPELINEOBJECTS_H
