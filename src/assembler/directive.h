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

class Directive {
public:
    using DirectiveHandler = std::function<HandleDirectiveRes(const AssemblerBase*, const TokenizedSrcLine&)>;
    Directive(const QString& directive, const DirectiveHandler& handler) : m_directive(directive), m_handler(handler) {}

    HandleDirectiveRes handle(const AssemblerBase* assembler, const TokenizedSrcLine& line) {
        return m_handler(assembler, line);
    }
    const QString& name() const { return m_directive; }

private:
    const QString m_directive;
    DirectiveHandler m_handler;
};

using DirectiveMap = std::map<QString, std::shared_ptr<Directive>>;
using DirectiveVec = std::vector<std::shared_ptr<Directive>>;

}  // namespace Assembler
}  // namespace Ripes
