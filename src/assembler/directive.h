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
    Directive(const QString& directive, const DirectiveHandler& handler, bool isEarly = false)
        : m_directive(directive), m_handler(handler), m_early(isEarly) {}

    HandleDirectiveRes handle(const AssemblerBase* assembler, const TokenizedSrcLine& line) {
        return m_handler(assembler, line);
    }
    const QString& name() const { return m_directive; }
    bool early() const { return m_early; }

private:
    const QString m_directive;
    DirectiveHandler m_handler;

    /**
     * @brief m_early
     * An early directive will be executed as soon as it is parsed
     */
    bool m_early;
};

using DirectiveMap = std::map<QString, std::shared_ptr<Directive>>;
using DirectiveVec = std::vector<std::shared_ptr<Directive>>;
using EarlyDirectives = std::set<QString>;
}  // namespace Assembler
}  // namespace Ripes
