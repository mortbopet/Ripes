#pragma once

#include <QByteArray>
#include <QString>
#include <functional>
#include <memory>
#include <variant>

#include "assembler_defines.h"
#include "assemblererror.h"

namespace Ripes {
namespace Assembler {

class AssemblerBase;

template <typename Reg_T>
using HandleRelocationRes = std::variant<Error, Reg_T>;

template <typename Reg_T>
class Relocation {
    static_assert(std::numeric_limits<Reg_T>::is_integer, "Register type must be integer");
    static_assert(std::numeric_limits<Instr_T>::is_integer, "Instruction type must be integer");

public:
    using RelocationHandler = std::function<HandleRelocationRes<Reg_T>(const Reg_T value, const Reg_T reloc_pos)>;
    Relocation(const QString& relocation, const RelocationHandler& handler)
        : m_relocation(relocation), m_handler(handler) {}

    HandleRelocationRes<Reg_T> handle(const Reg_T value, const Reg_T reloc_pos) { return m_handler(value, reloc_pos); }
    const QString& name() const { return m_relocation; }

private:
    const QString m_relocation;
    RelocationHandler m_handler;
};

template <typename Reg_T>
using RelocationsMap = std::map<QString, std::shared_ptr<Relocation<Reg_T>>>;

template <typename Reg_T>
using RelocationsVec = std::vector<std::shared_ptr<Relocation<Reg_T>>>;

}  // namespace Assembler
}  // namespace Ripes
