#pragma once

#include "directive.h"

namespace Ripes {
namespace Assembler {

/** @brief
 * A collection of standard GNU assembler directives
 */
/**
 * @brief gnuDirectives
 * @returns a collection of all of the available directives
 */
DirectiveVec gnuDirectives();

Directive zeroDirective();
Directive stringDirective();
Directive ascizDirective();

Directive doubleDirective();
Directive wordDirective();
Directive halfDirective();
Directive shortDirective();
Directive byteDirective();
Directive twoByteDirective();
Directive fourByteDirective();
Directive longDirective();
Directive alignDirective();

Directive dummyDirective(const QString& name);

Directive textDirective();
Directive dataDirective();
Directive bssDirective();
Directive equDirective();

}  // namespace Assembler
}  // namespace Ripes
