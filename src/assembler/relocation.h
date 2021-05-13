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

using HandleRelocationRes = std::variant<Error, uint32_t>;

class Relocation {
public:
    using RelocationHandler = std::function<HandleRelocationRes(const uint32_t symbol_addr, const uint32_t reloc_pos)>;
    Relocation(const QString& relocation, const RelocationHandler& handler)
        : m_relocation(relocation), m_handler(handler) {}

    HandleRelocationRes handle(const uint32_t symbol_addr, const uint32_t reloc_pos) {
        return m_handler(symbol_addr, reloc_pos);
    }
    const QString& name() const { return m_relocation; }

private:
    const QString m_relocation;
    RelocationHandler m_handler;
};

using RelocationsMap = std::map<QString, std::shared_ptr<Relocation>>;
using RelocationsVec = std::vector<std::shared_ptr<Relocation>>;

}  // namespace Assembler
}  // namespace Ripes
