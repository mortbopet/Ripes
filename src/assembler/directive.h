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

/// A directive argument consists of a tokenized source line as well as the program section of which the directive
/// handler should work on.
struct DirectiveArg {
    const TokenizedSrcLine& line;
    const ProgramSection* section;
};

/// An assembler directive represents a function which may be activated through the source code. This function will then
/// perform some transformation to the current state of the assembler.
class Directive {
public:
    using DirectiveHandler = std::function<HandleDirectiveRes(const AssemblerBase*, const DirectiveArg&)>;
    Directive(const QString& directive, const DirectiveHandler& handler, bool isEarly = false)
        : m_directive(directive), m_handler(handler), m_early(isEarly) {}

    /// Executes the directive handler.
    HandleDirectiveRes handle(const AssemblerBase* assembler, const DirectiveArg& arg) {
        return m_handler(assembler, arg);
    }
    const QString& name() const { return m_directive; }
    bool early() const { return m_early; }

private:
    /// Name of this directive.
    const QString m_directive;

    /// Directive handler.
    DirectiveHandler m_handler;

    /**
     * @brief m_early
     * An early directive will be executed as soon as it is parsed
     */
    bool m_early;
};

using DirectivePtr = std::shared_ptr<Directive>;
using DirectiveMap = std::map<QString, DirectivePtr>;
using DirectiveVec = std::vector<DirectivePtr>;
using EarlyDirectives = std::set<QString>;
}  // namespace Assembler
}  // namespace Ripes
